#include "FakeRequest.h"

TEST(AuthFilterTest, RouteExempt) {
    AuthFilter filter;
    auto req = makeRequest("/auth/login");
    crow::response res;

    filter.checkAuth(req, res);

    EXPECT_EQ(res.code, 200);
}

TEST(AuthFilterTest, RouteExempt2) {
    AuthFilter filter;
    auto req = makeRequest("/Auth/login");
    crow::response res;

    filter.checkAuth(req, res);

    EXPECT_EQ(res.code, 401);
}


TEST(AuthFilterTest, NoHeader) {
    AuthFilter filter;
    auto req = makeRequest("/Files");
    crow::response res;

    filter.checkAuth(req, res);

    EXPECT_EQ(res.code, 401);
    EXPECT_EQ(res.body, "Pas de Token");
    EXPECT_TRUE(res.is_completed());
}

TEST(AuthFilterTest, NoBearer) {
    AuthFilter filter;
    auto req = makeRequest("/Files", "abc123");
    crow::response res;

    filter.checkAuth(req, res);

    EXPECT_EQ(res.code, 401);
    EXPECT_EQ(res.body, "Pas de Token");
}

TEST(AuthFilterTest, JustBearer) {
    AuthFilter filter;
    auto req = makeRequest("/Files", "Bearer ");
    crow::response res;

    filter.checkAuth(req, res);

    EXPECT_EQ(res.code, 401);
    EXPECT_EQ(res.body, "Token invalide");
}

TEST(AuthFilterTest, NoClient) {
    AuthFilter filter;      
    auto req = makeRequest("/Files", "Bearer abc123");
    crow::response res;

    filter.checkAuth(req, res);

    EXPECT_EQ(res.code, 401);
    EXPECT_EQ(res.body, "Token invalide");
}

TEST(AuthFilterTest, ValidateTokenRefuse) {
    AuthFilter filter;
    std::string token;

    EXPECT_FALSE(filter.validateToken(token));
}