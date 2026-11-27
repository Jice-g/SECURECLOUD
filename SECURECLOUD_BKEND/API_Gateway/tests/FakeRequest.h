#pragma once

#include "crow.h"
#include <gtest/gtest.h>
#include <string>
#include "AuthFilter.h"

crow::request makeRequest(const std::string& url,const std::string& authorization = "")
{
    crow::request req;
    req.url = url;
    if (!authorization.empty()) {
        req.headers.emplace("Authorization", authorization);
    }
    return req;
}