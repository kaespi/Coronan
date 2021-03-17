#include "coronan/ssl_initializer.hpp"

#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/Net/SSLManager.h>

namespace coronan {

using Poco::Net::Context;

SSLInitializer::SSLInitializer(
    Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> certificate_handler,
    Poco::Net::Context::Ptr context)
    : certificate_handler_ptr{std::move(certificate_handler)},
      context_ptr{std::move(context)}
{
  Poco::Net::initializeSSL();
}

void SSLInitializer::initialize()
{
  Poco::Net::SSLManager::instance().initializeClient(
      nullptr, certificate_handler_ptr, context_ptr);
}

SSLInitializer::~SSLInitializer() { Poco::Net::uninitializeSSL(); }

std::unique_ptr<SSLInitializer>
SSLInitializer::initialize_with_accept_certificate_handler()
{
  constexpr auto private_key_file = "";
  constexpr auto certificate_file = "";
  constexpr auto ca_location = "";
  constexpr auto verification_mode = Context::VERIFY_RELAXED;
  constexpr auto verification_depth = 9;
  constexpr auto load_default_cas = false;
  constexpr auto cipher_list = "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH";
  constexpr auto handle_errors_on_server_side = false;

  Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> cert_handler =
      new Poco::Net::AcceptCertificateHandler{handle_errors_on_server_side};
  auto ssl_initializer = std::make_unique<SSLInitializer>(
      cert_handler, Context::Ptr{new Context{
                        Context::TLS_CLIENT_USE, private_key_file,
                        certificate_file, ca_location, verification_mode,
                        verification_depth, load_default_cas, cipher_list}});

  ssl_initializer->initialize();
  return ssl_initializer;
}

} // namespace coronan
