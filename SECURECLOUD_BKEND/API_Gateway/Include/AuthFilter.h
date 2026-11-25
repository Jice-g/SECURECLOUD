#pragma once

#include "crow.h"
#include <vector>
#include <string>
#include <memory> 

using namespace std;
class AuthClient;


class AuthFilter
{
	private:
		vector<string> routeExempt;
		string Jwt;
		std::shared_ptr<AuthClient> authClient;

	public:
		struct context{};

		AuthFilter();

		void setClient(std::shared_ptr<AuthClient> client);

		void before_handle(crow::request& req, crow::response& res, context& ctx);
		void after_handle(crow::request& req, crow::response& res, context& ctx);

		void checkAuth(crow::request& req, crow::response& res);
		bool validateToken(string&  token);



};