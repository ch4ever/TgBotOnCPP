#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <string>
#include <iostream>

using namespace std;
using nlohmann::json;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* response) {
    size_t totalSize = size * nmemb;
    response->append((char*)contents, totalSize);
    return totalSize;
}

string getCompletion(const string& prompt, const string& model) {
    string apiKey = "TOKEN"; 
    string baseUrl = "https://api.openai.com/v1/chat/completions";
    string response;
    CURL* curl = curl_easy_init();

    if (curl) {
        json requestData = {
            {"model", model},
            {"messages", {{{"role", "user"}, {"content", prompt}}}},
            {"temperature", 0.7},
            {"max_tokens", 50}
        };
        string requestDataStr = requestData.dump();

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, ("Authorization: Bearer " + apiKey).c_str());

        curl_easy_setopt(curl, CURLOPT_URL, baseUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestDataStr.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, requestDataStr.size());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);

       
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        cout << "HTTP Status Code: " << httpCode << endl; 

        if (res != CURLE_OK) {
            cerr << "Curl request failed: " << curl_easy_strerror(res) << endl;
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
            return "Error: Request failed";
        }

        if (httpCode != 200) {
            cerr << "HTTP Error: " << httpCode << endl;  
            cerr << "Response: " << response << endl;  
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
            return "Error: HTTP request failed";
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    else {
        cerr << "Failed to initialize CURL" << endl;
        return "Error: CURL initialization failed";
    }

    json jresponse;
    try {
        jresponse = json::parse(response);
        if (jresponse.contains("choices") && !jresponse["choices"].empty() &&
            jresponse["choices"][0].contains("message") &&
            jresponse["choices"][0]["message"].contains("content")) {
            return jresponse["choices"][0]["message"]["content"].get<string>();
        }
        else {
            cerr << "Unexpected JSON structure: " << jresponse.dump(4) << endl;
            return "Error: Unexpected JSON structure";
        }
    }
    catch (const json::parse_error& e) {
        cerr << "JsonParseError: " << e.what() << endl;
        return "Error: Unable to parse JSON response";
    }
}
