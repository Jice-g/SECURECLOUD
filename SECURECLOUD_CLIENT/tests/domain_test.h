#ifndef DOMAIN_TEST_H
#define DOMAIN_TEST_H

#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>



// =============================================================================
// Minimal test harness (no dependency).
//
// Why not <cassert>?  assert() is compiled OUT when NDEBUG is defined (Release /
// CI builds): the tests would then "pass" without checking anything.
// CHECK* below are always active, do not abort on first failure, and report the
// file/line of every failed check.
// =============================================================================

// test tables :
static std::vector<std::string> g_passedTests;
static std::vector<std::string> g_failedTests;
// test counters :
static int g_checksInCurrentTest = 0;
static int g_failuresInCurrentTest = 0;
static int g_totalChecks = 0;



/***********************************************************
*
*    Macro that uses the GoogleTest logical_and
*
************************************************************/

/// CHECK la condition passée en paramètre
#define CHECK(cond)                                                              \
    do {                                                                         \
        ++g_checksInCurrentTest;											     \
		++g_totalChecks;                                                         \
        if (!(cond)) {                                                           \   ++g_failuresInCurrentTest;                                               \
            std::cerr << "\n    CHECK FAILED " << __FILE__ << ":" << __LINE__    \
                      << "  " << #cond << std::endl;                             \
        }                                                                        \
    } while (0)

/// Passes only if `expr` throws an exception of type `ExType`.
#define CHECK_THROWS(expr, ExType)                                               \
    do {                                                                         \
        ++g_checksInCurrentTest;                                                 \
		++g_totalChecks;                                                         \
        bool threwExpected_ = false;                                             \
        try { (void)(expr); }                                                    \
        catch (const ExType&) { threwExpected_ = true; }                         \
        catch (...) {}                                                           \
        if (!threwExpected_) {                                                   \
            ++g_failuresInCurrentTest;                                           \
            std::cerr << "\n    CHECK_THROWS FAILED " << __FILE__ << ":"         \
                      << __LINE__ << "  " << #expr << " (expected " #ExType ")"  \
                      << std::endl;                                              \
        }                                                                        \
    } while (0)

/// Passes only if `expr` does NOT throw.
#define CHECK_NOTHROW(expr)                                                      \
    do {                                                                         \
        ++g_checksInCurrentTest; ++g_totalChecks;                                \
        try { (void)(expr); }                                                    \
        catch (...) {                                                            \
            ++g_failuresInCurrentTest;                                           \
            std::cerr << "\n    CHECK_NOTHROW FAILED " << __FILE__ << ":"        \
                      << __LINE__ << "  " << #expr << std::endl;                 \
        }                                                                        \
    } while (0)

/// Runs a test function, records pass/fail, survives unexpected exceptions.
#define RUN_TEST(testName)                                                       \
    do {                                                                         \
        std::cout << #testName << " ..." << std::flush;                          \
        g_checksInCurrentTest = 0; g_failuresInCurrentTest = 0;                  \
        try { testName(); }                                                      \
        catch (const std::exception& e_) {                                       \
            ++g_failuresInCurrentTest;                                           \
            std::cerr << "\n    UNEXPECTED EXCEPTION: " << e_.what();            \
        } catch (...) {                                                          \
            ++g_failuresInCurrentTest;                                           \
            std::cerr << "\n    UNEXPECTED UNKNOWN EXCEPTION";                   \
        }                                                                        \
        if (g_failuresInCurrentTest == 0) {                                      \
            std::cout << " [OK] (" << g_checksInCurrentTest << " checks)\n";     \
            g_passedTests.push_back(#testName);                                  \
        } else {                                                                 \
            std::cout << " [FAILED]\n";                                          \
            g_failedTests.push_back(#testName);                                  \
        }                                                                        \
    } while (0)



/// Writes the report (passed AND failed tests). Path comes from the caller.
///
static void writeReport(const std::string& filePath, const std::string& title)
{
    std::ofstream report(filePath);
    if (!report) {
        std::cerr << "[ERROR] Could not open report file: " << filePath << "\n";
        return;
    }
	
    report << "=====================================================\n"
           << "SecureCloud Client - " << title << "\n"
           << "=====================================================\n\n"
           << "Tests passed : " << g_passedTests.size() << "\n"
           << "Tests failed : " << g_failedTests.size() << "\n"
           << "Total checks : " << g_totalChecks << "\n\n";
		   
    for (const auto& passed : g_passedTests) report << "  [OK]     " << passed << "\n";
    for (const auto& failed : g_failedTests) report << "  [FAILED] " << failed << "\n";
    report << "\nEnd of report.\n";
    std::cout << "[REPORT] Written to: " << filePath << "\n";
}

#endif // DOMAIN_TEST_H
