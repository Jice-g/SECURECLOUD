#include "RouteurTest.h"

void Router::registerRoutes(crow::App<AuthFilter>& app) {

	//Route Teste
	CROW_ROUTE(app, "/TestAPI")
		([]()
			{
				return crow::response(200, "API Gateway OK");
			}
			);

	//Route for files upload
	CROW_ROUTE(app, "/Files").methods(crow::HTTPMethod::POST)
		([](const crow::request& files)
			{
				return crow::response(200, "Files OK");
			}
			);

	//Route for open WebSocjet
	CROW_ROUTE(app, "/WebSocket").websocket(&app).onopen
	([](crow::websocket::connection& connexion)
		{
			CROW_LOG_INFO << "Coonnexion de la WebSocket";
		})

		.onmessage
		([](crow::websocket::connection& connexion, const std::string& text, bool)
			{
				connexion.send_text("Texte envoyer : " + text);
			})
		.onclose
		([](crow::websocket::connection& connexion, const std::string& why, auto&&...)
			{
				CROW_LOG_INFO << "Connexion ferme";
			});
}