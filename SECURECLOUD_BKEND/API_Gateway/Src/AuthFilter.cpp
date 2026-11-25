#include "AuthFilter.h"
#include "AuthClient.h"

AuthFilter::AuthFilter() : Jwt("JWT") {

	routeExempt = { "/TestAPI" };
}

void AuthFilter::setClient(std::shared_ptr<AuthClient> client) {
	authClient = std::move(client);
}

void AuthFilter::before_handle(crow::request& req, crow::response& res, context& ctx) {
	checkAuth(req, res);
}

void AuthFilter::after_handle(crow::request& req, crow::response& res, context& ctx) {
	
}

void AuthFilter::checkAuth(crow::request& req, crow::response& res) {
	for (const string& route : routeExempt)
	{
		if (req.url == route)
		{
			return;
		}
	}

	string header = req.get_header_value("Authorization");
	const string prefix = "Bearer ";

	if (header.rfind(prefix, 0) != 0) {
		res.code = 401;
		res.write("Pas de Token");
		res.end();
		return;
	}

	string token = header.substr(prefix.size());

	if (!validateToken(token)) {
		res.code = 401;
		res.write("Token invalide");
		res.end();
		return;
	}

}

bool AuthFilter::validateToken(string& token) {
	if (token.empty() || !authClient) {
		return false;
	}
	return authClient->validateToken(token);
}
	
