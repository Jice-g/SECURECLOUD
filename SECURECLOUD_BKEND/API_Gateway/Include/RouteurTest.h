#pragma once
#include "crow.h"
#include "AuthFilter.h"

class Router 
{
	public : 
		void registerRoutes(crow::App<AuthFilter>& app);
};
