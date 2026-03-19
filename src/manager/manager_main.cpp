#include "manager_service.h"

#include <grpcpp/grpcpp.h>

#include <iostream>

int main(int argc, char** argv) {
  std::string listen_addr = "0.0.0.0:50051";
  std::string db_path = "metrics.db";

  if (argc > 1) {
    listen_addr = argv[1];
  }
  if (argc > 2) {
    db_path = argv[2];
  }

  dist::sys::monitor::ManagerServiceImpl service(db_path);

  grpc::ServerBuilder builder;
  builder.AddListeningPort(listen_addr, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "manager listening on " << listen_addr << "\n";
  server->Wait();
  return 0;
}
