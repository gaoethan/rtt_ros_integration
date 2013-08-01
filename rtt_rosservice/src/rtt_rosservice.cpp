
#include <rtt/RTT.hpp>
#include <rtt/plugin/ServicePlugin.hpp>
#include <rtt/internal/GlobalService.hpp>
#include <rtt_rostopic/rtt_rostopic.h> 

using namespace RTT;
using namespace std;

/**
 * The globally loadable ROS service.
 */
class ROSServiceService : public RTT::Service {
public:
  /**
   * Instantiates this service.
   * @param owner The owner or null in case of global.
   */
  ROSTopicService(TaskContext* owner) 
    : Service("rosservice", owner)
  {
    this->doc("Main RTT Service for connecting RTT operations to ROS service clients and servers.");

    this->provides("clients");
    this->provides("servers");

    this->addOperation("client", &ROSServiceService::client, this).doc(
        "Creates a ROS service client and an associated Orocos operation.").arg(
          "name", "The ros service name.");
    this->addOperation("server", &ROSServiceService::server, this).doc(
        "Creates a ROS service server and an associated Orocos operation caller").arg(
          "name", "The ros service name.");

    this->addOperation("registerServiceType", &ROSServiceService::registerServiceType, this);

  }

  void registerServiceType(ROSServiceProxyFactoryBase* factory) {
    // Get the package name and service type
    std::string service_package = factory->getPackage();
    std::string service_type = factory->getType();

    // Store the factory
    factories_[service_package][service_type] = factory;
  }

  /** \brief Add an Orocos operation to this service which calls a ROS service
   * client.
   *
   * This allows another component to call this operation in order to invoke a
   * ROS service call.
   */
  const std::string client(
    const std::string &ros_service_name,
    const std::string &ros_service_type)
  {
    // Check if the client proxy already exists
    if(client_proxies_.find(ros_service_name) != client_proxies_.end()) {
      return client_proxies_[ros_service_name]->getOperationName();
    }
        
    // Split the typename
    std::vector<std::string> type_tokens;
    boost::split(type_tokens, ros_service_type, boost::is_any_of("/"));
    if(type_tokens.size() != 2) { return "ERROR"; }

    // Have the factory create a ROS service client add the operation 
    ROSServiceClientProxyBase* client_proxy =
      factories_[type_tokens[0]][type_tokens[1]]->create_client_proxy(this, ros_service_name);
    // Store the client proxy
    client_proxies_[ros_service_name] = client_proxy;

    return client_proxy->getOperationName();
  }

  /** \brief Add an Orocos operation caller to this service which is called by
   * a ROS service server.
   *
   * This allows another component to be called by this service when a ROS
   * service call is invoked.
   */
  const std::string server(const std::string &ros_service_name) {
    // create operationcaller

    return "";
  }


  std::vector<ros::ServiceClient> clients_;
  std::vector<ros::ServiceServer> servers_;

};

void loadROSServiceService(){
  RTT::Service::shared_ptr rss(new ROSServiceService(0));
  RTT::internal::GlobalService::Instance()->addService(rss);
}

using namespace RTT;
extern "C" {
  bool loadRTTPlugin(RTT::TaskContext* c){
    loadROSServiceService();
    return true;
  }
  std::string getRTTPluginName (){
    return "rosservice";
  }
  std::string getRTTTargetName (){
    return OROCOS_TARGET_NAME;
  }
}
