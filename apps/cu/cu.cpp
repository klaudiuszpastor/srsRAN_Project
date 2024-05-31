/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "srsran/f1ap/gateways/f1c_local_connector_factory.h"
#include "srsran/f1u/cu_up/split_connector/f1u_split_connector.h"
#include "srsran/gateways/udp_network_gateway.h"
#include "srsran/gtpu/gtpu_config.h"
#include "srsran/gtpu/gtpu_demux_factory.h"
#include "srsran/gtpu/ngu_gateway.h"
#include "srsran/pcap/dlt_pcap.h"
#include "srsran/support/backtrace.h"
#include "srsran/support/build_info/build_info.h"
#include "srsran/support/config_parsers.h"
#include "srsran/support/cpu_features.h"
#include "srsran/support/error_handling.h"
#include "srsran/support/event_tracing.h"
#include "srsran/support/io/io_broker.h"
#include "srsran/support/io/io_broker_factory.h"
#include "srsran/support/signal_handler.h"
#include "srsran/support/sysinfo.h"
#include "srsran/support/timers.h"
#include "srsran/support/version/version.h"

#include "apps/cu/cu_appconfig_cli11_schema.h"
#include "apps/cu/cu_worker_manager.h"
#include "apps/services/metrics_log_helper.h"
#include "apps/units/cu_cp/cu_cp_builder.h"
#include "apps/units/cu_cp/cu_cp_logger_registrator.h"
#include "apps/units/cu_cp/cu_cp_unit_config.h"
#include "apps/units/cu_cp/cu_cp_unit_config_cli11_schema.h"
#include "apps/units/cu_cp/cu_cp_unit_config_validator.h"
#include "apps/units/cu_cp/cu_cp_unit_logger_config.h"
#include "apps/units/cu_up/cu_up_logger_registrator.h"
#include "apps/units/cu_up/cu_up_unit_config.h"
#include "apps/units/cu_up/cu_up_unit_config_cli11_schema.h"
#include "apps/units/cu_up/cu_up_unit_config_validator.h"

// TODO remove apps/gnb/*.h
#include "apps/gnb/adapters/e1ap_gateway_local_connector.h"
#include "apps/gnb/adapters/e2_gateway_remote_connector.h"
#include "apps/gnb/adapters/ngap_adapter.h"
#include "apps/gnb/gnb_appconfig_translators.h"

#include "cu_appconfig.h"

#include <atomic>
#include <thread>

using namespace srsran;

/// \file
/// \brief Application of a Central Unit (CU) with combined CU control-plane (CU-CP) and CU user-plane (CU-UP).
///
/// This application runs a CU without the E1 connection between the CU-CP and CU-UP going over a real SCTP
/// connection. However, its does expose the F1, N2 and N3 interface to the DU, AMF and UPF over the standard
/// UDP/SCTP ports.
///
/// The app serves as an example for an all-integrated CU.
///
/// \cond

static std::string config_file;

static std::atomic<bool> is_running = {true};
const int                MAX_CONFIG_FILES(10);

static void populate_cli11_generic_args(CLI::App& app)
{
  fmt::memory_buffer buffer;
  format_to(buffer, "srsRAN 5G CU version {} ({})", get_version(), get_build_hash());
  app.set_version_flag("-v,--version", srsran::to_c_str(buffer));
  app.set_config("-c,", config_file, "Read config from file", false)->expected(1, MAX_CONFIG_FILES);
}

static void local_signal_handler()
{
  is_running = false;
}

static void initialize_log(const std::string& filename)
{
  srslog::sink* log_sink = (filename == "stdout") ? srslog::create_stdout_sink() : srslog::create_file_sink(filename);
  if (log_sink == nullptr) {
    report_error("Could not create application main log sink.\n");
  }
  srslog::set_default_sink(*log_sink);
  srslog::init();
}

static void register_app_logs(const log_appconfig&            log_cfg,
                              const cu_cp_unit_logger_config& cu_cp_loggers,
                              const cu_up_unit_logger_config& cu_up_loggers)
{
  // Set log-level of app and all non-layer specific components to app level.
  for (const auto& id : {"CU", "ALL", "SCTP-GW", "IO-EPOLL", "UDP-GW", "PCAP"}) {
    auto& logger = srslog::fetch_basic_logger(id, false);
    logger.set_level(srslog::str_to_basic_level(log_cfg.lib_level));
    logger.set_hex_dump_max_size(log_cfg.hex_max_size);
  }

  auto& config_logger = srslog::fetch_basic_logger("CONFIG", false);
  config_logger.set_level(srslog::str_to_basic_level(log_cfg.config_level));
  config_logger.set_hex_dump_max_size(log_cfg.hex_max_size);

  auto& metrics_logger = srslog::fetch_basic_logger("METRICS", false);
  metrics_logger.set_level(srslog::str_to_basic_level(log_cfg.metrics_level));
  metrics_logger.set_hex_dump_max_size(log_cfg.hex_max_size);

  auto& e2ap_logger = srslog::fetch_basic_logger("E2AP", false);
  e2ap_logger.set_level(srslog::str_to_basic_level(log_cfg.e2ap_level));
  e2ap_logger.set_hex_dump_max_size(log_cfg.hex_max_size);

  // Register units logs.
  register_cu_cp_loggers(cu_cp_loggers);
  register_cu_up_loggers(cu_up_loggers);
}

int main(int argc, char** argv)
{
  // Set signal handler.
  register_signal_handler(local_signal_handler);

  // Enable backtrace.
  enable_backtrace();

  // Setup and configure config parsing.
  CLI::App app("srsCU application");
  app.config_formatter(create_yaml_config_parser());
  app.allow_config_extras(CLI::config_extras_mode::error);
  // Fill the generic application arguments to parse.
  populate_cli11_generic_args(app);

  // Configure CLI11 with the CU application configuration schema.
  cu_appconfig cu_cfg;
  configure_cli11_with_cu_appconfig_schema(app, cu_cfg);

  cu_cp_unit_config cu_cp_config;
  configure_cli11_with_cu_cp_unit_config_schema(app, cu_cp_config);

  cu_up_unit_config cu_up_config;
  configure_cli11_with_cu_up_unit_config_schema(app, cu_up_config);

  // Set the callback for the app calling all the autoderivation functions.
  app.callback([&app, &cu_cp_config]() {
    // Create the PLMN and TAC list from the cells.
    // TODO remove hard-coded value
    std::vector<std::string> plmns;
    std::vector<unsigned>    tacs;
    plmns.emplace_back("00101");
    tacs.emplace_back(7);

    autoderive_cu_cp_parameters_after_parsing(app, cu_cp_config, std::move(plmns), std::move(tacs));
  });

  // Parse arguments.
  CLI11_PARSE(app, argc, argv);

  // Check the modified configuration.
  if (!validate_cu_cp_unit_config(cu_cp_config) || !validate_cu_up_unit_config(cu_up_config)) {
    report_error("Invalid configuration detected.\n");
  }

  // Set up logging.
  initialize_log(cu_cfg.log_cfg.filename);
  register_app_logs(cu_cfg.log_cfg, cu_cp_config.loggers, cu_up_config.loggers);

  // Log input configuration.
  srslog::basic_logger& config_logger = srslog::fetch_basic_logger("CONFIG");
  if (config_logger.debug.enabled()) {
    config_logger.debug("Input configuration (all values): \n{}", app.config_to_str(true, false));
  } else {
    config_logger.info("Input configuration (only non-default values): \n{}", app.config_to_str(false, false));
  }

  srslog::basic_logger& cu_logger = srslog::fetch_basic_logger("CU");
  if (not cu_cfg.log_cfg.tracing_filename.empty()) {
    cu_logger.info("Opening event tracer...");
    open_trace_file(cu_cfg.log_cfg.tracing_filename);
    cu_logger.info("Event tracer opened successfully");
  }

  // configure cgroups
  // TODO

  // Setup size of byte buffer pool.
  // TODO byte_buffer_pool

  // Log build info
  cu_logger.info("Built in {} mode using {}", get_build_mode(), get_build_info());

  // Log CPU architecture.
  // TODO

  // Check and log included CPU features and check support by current CPU
  if (cpu_supports_included_features()) {
    cu_logger.debug("Required CPU features: {}", get_cpu_feature_info());
  } else {
    // Quit here until we complete selection of the best matching implementation for the current CPU at runtime.
    cu_logger.error("The CPU does not support the required CPU features that were configured during compile time: {}",
                    get_cpu_feature_info());
    report_error("The CPU does not support the required CPU features that were configured during compile time: {}\n",
                 get_cpu_feature_info());
  }

  // Check some common causes of performance issues and print a warning if required.
  check_cpu_governor(cu_logger);
  check_drm_kms_polling(cu_logger);

  // Create worker manager.
  cu_worker_manager workers{cu_cfg, cu_up_config.gtpu_queue_size};

  // Create layer specific PCAPs.
  // TODO:
  // 1. modules::...create_pcap does not use the custom cu_worker.
  // 2. modules::flexible_du... for creating F1AP pcap.
  // Initializing PCAPs direclty.
  std::unique_ptr<dlt_pcap>              ngap_p = create_null_dlt_pcap();
  std::vector<std::unique_ptr<dlt_pcap>> cu_up_pcaps(2);
  cu_up_pcaps[0]                   = create_null_dlt_pcap();
  cu_up_pcaps[1]                   = create_null_dlt_pcap();
  std::unique_ptr<dlt_pcap> f1ap_p = create_null_dlt_pcap();
  std::unique_ptr<dlt_pcap> e2ap_p =
      cu_cfg.cu_cp_pcap_cfg.e2ap.enabled
          ? create_e2ap_pcap(cu_cfg.cu_cp_pcap_cfg.e2ap.filename, workers.get_executor("pcap_exec"))
          : create_null_dlt_pcap();

  // Create IO broker.
  const auto&                low_prio_cpu_mask = cu_cfg.expert_execution_cfg.affinities.low_priority_cpu_cfg.mask;
  io_broker_config           io_broker_cfg(low_prio_cpu_mask);
  std::unique_ptr<io_broker> epoll_broker = create_io_broker(io_broker_type::epoll, io_broker_cfg);

  // Create F1-C GW (TODO pass actual arguments for F1AP IPs)
  f1c_local_sctp_connector_config      f1c_conn_cfg({*ngap_p, *epoll_broker});
  std::unique_ptr<f1c_local_connector> cu_f1c_gw = srsran::create_f1c_local_connector(f1c_conn_cfg);

  // Create F1-U GW (TODO factory and cleanup).
  gtpu_demux_creation_request cu_f1u_gtpu_msg   = {};
  cu_f1u_gtpu_msg.cfg.warn_on_drop              = true;
  cu_f1u_gtpu_msg.gtpu_pcap                     = cu_up_pcaps[1].get(); // FIXME use right enum
  std::unique_ptr<gtpu_demux> cu_f1u_gtpu_demux = create_gtpu_demux(cu_f1u_gtpu_msg);
  udp_network_gateway_config  cu_f1u_gw_config  = {};
  cu_f1u_gw_config.bind_address                 = "127.0.2.1";
  cu_f1u_gw_config.bind_port                    = GTPU_PORT;
  cu_f1u_gw_config.reuse_addr                   = true;
  std::unique_ptr<srs_cu_up::ngu_gateway> cu_f1u_gw =
      srs_cu_up::create_udp_ngu_gateway(cu_f1u_gw_config, *epoll_broker, *workers.cu_up_io_ul_exec);
  std::unique_ptr<srs_cu_up::f1u_split_connector> cu_f1u_conn =
      std::make_unique<srs_cu_up::f1u_split_connector>(cu_f1u_gw.get(), cu_f1u_gtpu_demux.get(), *cu_up_pcaps[1].get());

  // Create E1AP local connector
  e1ap_gateway_local_connector e1ap_gw{*cu_up_pcaps[0]};

  // Create manager of timers for CU-CP and CU-UP, which will be
  // driven by the system timer slot ticks.
  // TODO revisit how to use the system timer timer source.
  timer_manager  app_timers{256};
  timer_manager* cu_timers = &app_timers;

  // Set up the JSON log channel used by metrics.
  // TODO metrics. Do we have any CU-CP or CU-UP JSON metrics?

  // Create NGAP Gateway.
  // TODO had to include gnb
  std::unique_ptr<srs_cu_cp::ngap_gateway_connector> ngap_adapter;
  {
    using no_core_mode_t = srs_cu_cp::ngap_gateway_params::no_core;
    using network_mode_t = srs_cu_cp::ngap_gateway_params::network;
    using ngap_mode_t    = variant<no_core_mode_t, network_mode_t>;

    // TODO generate network config in helper function, not in apps/gnb
    srsran::sctp_network_connector_config n2_nw_cfg;
    n2_nw_cfg.connection_name = "AMF";
    n2_nw_cfg.connect_address = cu_cp_config.amf_cfg.ip_addr;
    n2_nw_cfg.connect_port    = cu_cp_config.amf_cfg.port;
    if (cu_cp_config.amf_cfg.n2_bind_addr == "auto") {
      n2_nw_cfg.bind_address = cu_cp_config.amf_cfg.bind_addr;
    } else {
      n2_nw_cfg.bind_address = cu_cp_config.amf_cfg.n2_bind_addr;
    }
    n2_nw_cfg.bind_interface = cu_cp_config.amf_cfg.n2_bind_interface;
    n2_nw_cfg.ppid           = NGAP_PPID;

    ngap_adapter = srs_cu_cp::create_ngap_gateway(srs_cu_cp::ngap_gateway_params{
        *ngap_p,
        cu_cp_config.amf_cfg.no_core ? ngap_mode_t{no_core_mode_t{}}
                                     : ngap_mode_t{network_mode_t{*epoll_broker, n2_nw_cfg}}});
  }

  // E2AP configuration.
  // Create E2AP GW remote connector.
  // TODO This seems to be used in the DU only?

  // Create CU-CP config.
  cu_cp_build_dependencies cu_cp_dependencies;
  cu_cp_dependencies.cu_cp_executor = workers.cu_cp_exec;
  cu_cp_dependencies.cu_cp_e2_exec  = workers.cu_cp_e2_exec;
  cu_cp_dependencies.ngap_notifier  = ngap_adapter.get();
  cu_cp_dependencies.timers         = cu_timers;

  // create CU-CP.
  std::unique_ptr<srsran::srs_cu_cp::cu_cp> cu_cp_obj = build_cu_cp(cu_cp_config, cu_cp_dependencies);

  // Create console helper object for commands and metrics printing.
  // TODO console helper

  // Create metrics log helper.
  metrics_log_helper metrics_logger(srslog::fetch_basic_logger("METRICS"));

  // Connect NGAP adpter to CU-CP to pass NGAP messages.
  ngap_adapter->connect_cu_cp(cu_cp_obj->get_ng_handler().get_ngap_message_handler(),
                              cu_cp_obj->get_ng_handler().get_ngap_event_handler());

  // Connect E1AP to CU-CP.
  e1ap_gw.attach_cu_cp(cu_cp_obj->get_e1_handler());

  // Connect F1-C to CU-CP.
  cu_f1c_gw->attach_cu_cp(cu_cp_obj->get_f1c_handler());

  // start CU-CP
  cu_logger.info("Starting CU-CP...");
  cu_cp_obj->start();
  cu_logger.info("CU-CP started successfully");

  // Check connection to AMF
  // TODO

  // Create and start CU-UP
  // TODO cu_up

  // Move all the DLT PCAPs to a container.
  // TODO

  // Start processing.
  // TODO console.on_app_running()

  while (is_running) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // Console helper print stop
  // TODO

  // Stop CU-UP activity.
  // TODO

  // Stop CU-CP activity.
  // TODO

  // Close network connections
  // TODO

  // Close PCAPs
  // TODO

  // Stop workers
  // TODO

  srslog::flush();

  if (not cu_cfg.log_cfg.tracing_filename.empty()) {
    cu_logger.info("Closing event tracer...");
    close_trace_file();
    cu_logger.info("Event tracer closed successfully");
  }

  // Clean cgroups
  // TODO required for CU?

  return 0;
}

/// \endcond
