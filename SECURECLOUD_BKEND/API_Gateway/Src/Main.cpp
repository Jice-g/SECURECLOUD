#include <crow.h>
#include "RouteurTest.h"
#include "AuthFilter.h"
#include "AuthClient.h"
#include "AuthController.h"

int main()
{
	crow::App<AuthFilter> app;

	auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
	auto authClient = std::make_shared<AuthClient>(channel);
	app.get_middleware<AuthFilter>().setClient(authClient);

	AuthController authController(*authClient);
	authController.registerRoutes(app);

	Router router;
	router.registerRoutes(app);

	app.port(8085);
	app.multithreaded();
	app.run();
}