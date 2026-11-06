/*
 *
 * Copyright Marc Ketel
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "WebConfigStore.h"

#include <Arduino.h>
#include <Preferences.h>
#include <esp_system.h>

#include <cstring>

namespace {
    const char *skipWhitespace(const char *p) {
        while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t' || *p == ',') {
            p++;
        }
        return p;
    }

    struct FieldDesc {
        const char *name;
        size_t nameLen;
        char *outBuf;
        size_t outBufSize;
    };

    // Parse one key/value field from a JSON object.
    const char *parseOneField(const char *p, const FieldDesc *fields, size_t fieldCount) {
        p = skipWhitespace(p);
        if (*p == '}' || *p == '\0') {
            return p;
        }

        if (*p != '"') {
            return nullptr;
        }
        p++;

        const char *keyStart = p;
        while (*p && *p != '"') {
            p++;
        }
        if (*p != '"') {
            return nullptr;
        }
        size_t keyLen = p - keyStart;
        p++;

        p = skipWhitespace(p);
        if (*p != ':') {
            return nullptr;
        }
        p++;
        p = skipWhitespace(p);

        if (*p != '"') {
            return nullptr;
        }
        p++;

        const char *valStart = p;
        while (*p && *p != '"') {
            p++;
        }
        if (*p != '"') {
            return nullptr;
        }
        size_t valLen = p - valStart;
        p++;

        for (size_t i = 0; i < fieldCount; i++) {
            if (keyLen == fields[i].nameLen && strncmp(keyStart, fields[i].name, fields[i].nameLen) == 0) {
                size_t copyLen = valLen < fields[i].outBufSize - 1 ? valLen : fields[i].outBufSize - 1;
                memcpy(fields[i].outBuf, valStart, copyLen);
                fields[i].outBuf[copyLen] = '\0';
                break;
            }
        }

        return p;
    }
}

namespace web_config {
    size_t parseNetworkArray(const char *json, Network *out, size_t maxCount) {
        if (json == nullptr || out == nullptr || maxCount == 0) {
            return 0;
        }

        const char *arr = strchr(json, '[');
        if (arr == nullptr) {
            return 0;
        }

        const char *p = arr + 1;
        size_t count = 0;

        while (*p && count < maxCount) {
            p = skipWhitespace(p);

            if (*p == ']' || *p == '\0') {
                break;
            }

            if (*p != '{') {
                return count;
            }
            p++;

            char ssid[SSID_LEN] = {};
            char pass[PASS_LEN] = {};

            FieldDesc fields[] = {
                {"ssid", 4, ssid, sizeof(ssid)},
                {"pass", 4, pass, sizeof(pass)},
            };

            bool valid = true;
            while (true) {
                p = parseOneField(p, fields, 2);
                if (p == nullptr) {
                    valid = false;
                    break;
                }
                if (*p == '}') {
                    p++;
                    break;
                }
                if (*p == '\0') {
                    valid = false;
                    break;
                }
            }

            if (valid && ssid[0] != '\0') {
                memset(&out[count], 0, sizeof(Network));
                snprintf(out[count].ssid, sizeof(out[count].ssid), "%s", ssid);
                snprintf(out[count].pass, sizeof(out[count].pass), "%s", pass);
                count++;
            }

            p = skipWhitespace(p);
        }

        return count;
    }

    int sanitizeNetworksBody(const char *rawBody, char *outJson, size_t outSize) {
        if (rawBody == nullptr || outJson == nullptr || outSize == 0) {
            return 0;
        }

        Network *networks = new Network[MAX_NETWORKS];
        size_t count = parseNetworkArray(rawBody, networks, MAX_NETWORKS);

        if (count == 0) {
            delete[] networks;
            return 0;
        }

        size_t pos = 0;
        int written = snprintf(outJson, outSize, "{\"networks\":[");
        if (written < 0 || static_cast<size_t>(written) >= outSize) {
            delete[] networks;
            return 0;
        }
        pos = static_cast<size_t>(written);

        for (size_t i = 0; i < count; i++) {
            written = snprintf(outJson + pos, outSize - pos, "%s{\"ssid\":\"%s\",\"pass\":\"%s\"}",
                               i > 0 ? "," : "", networks[i].ssid, networks[i].pass);
            if (written < 0 || static_cast<size_t>(written) >= outSize - pos) {
                delete[] networks;
                return 0;
            }
            pos += static_cast<size_t>(written);
        }

        written = snprintf(outJson + pos, outSize - pos, "]}");
        if (written < 0 || static_cast<size_t>(written) >= outSize - pos) {
            delete[] networks;
            return 0;
        }

        delete[] networks;
        return static_cast<int>(count);
    }

    size_t loadNetworks(Network *out, size_t maxCount) {
        if (out == nullptr || maxCount == 0) {
            return 0;
        }

        Preferences preferences;
        if (!preferences.begin(NVS_NAMESPACE, true)) {
            return 0;
        }

        String json = preferences.getString(NVS_KEY_WIFI_CFG, "");
        preferences.end();

        if (json.length() == 0) {
            return 0;
        }

        return parseNetworkArray(json.c_str(), out, maxCount);
    }

    bool saveNetworksJson(const char *json) {
        if (json == nullptr) {
            return false;
        }

        Preferences preferences;
        if (!preferences.begin(NVS_NAMESPACE, false)) {
            return false;
        }

        size_t written = preferences.putString(NVS_KEY_WIFI_CFG, json);
        preferences.end();

        return written == strlen(json);
    }

    bool loadApPassword(char *out, size_t outSize) {
        if (out == nullptr || outSize == 0) {
            return false;
        }

        Preferences preferences;
        if (!preferences.begin(NVS_NAMESPACE, true)) {
            return false;
        }

        String pass = preferences.getString(NVS_KEY_AP_PASS, "");
        preferences.end();

        if (pass.length() == 0) {
            return false;
        }

        snprintf(out, outSize, "%s", pass.c_str());
        return true;
    }

    bool saveApPassword(const char *pass) {
        if (pass == nullptr) {
            return false;
        }

        Preferences preferences;
        if (!preferences.begin(NVS_NAMESPACE, false)) {
            return false;
        }

        size_t written = preferences.putString(NVS_KEY_AP_PASS, pass);
        preferences.end();

        return written == strlen(pass);
    }

    void generateApPassword(char *out, size_t outSize) {
        if (out == nullptr || outSize < AP_PASS_LEN) {
            return;
        }

        for (size_t i = 0; i < 8; i++) {
            out[i] = static_cast<char>('a' + esp_random() % 26U);
        }
        for (size_t i = 8; i < 12; i++) {
            out[i] = static_cast<char>('0' + esp_random() % 10U);
        }
        out[12] = '\0';
    }

    bool ensureApPassword(char *out, size_t outSize) {
        if (loadApPassword(out, outSize)) {
            return true;
        }

        char pass[AP_PASS_LEN];
        generateApPassword(pass, sizeof(pass));

        if (!saveApPassword(pass)) {
            return false;
        }

        snprintf(out, outSize, "%s", pass);
        return true;
    }

    void escapeJson(const char *in, char *out, size_t outSize) {
        if (out == nullptr || outSize == 0) {
            return;
        }

        size_t pos = 0;
        for (const char *p = in; p != nullptr && *p != '\0' && pos + 1 < outSize; p++) {
            unsigned char c = static_cast<unsigned char>(*p);

            if (c == '"' || c == '\\') {
                if (pos + 2 >= outSize) {
                    break;
                }
                out[pos++] = '\\';
                out[pos++] = static_cast<char>(c);
            } else if (c < 0x20) {
                if (pos + 6 >= outSize) {
                    break;
                }
                snprintf(out + pos, outSize - pos, "\\u%04x", c);
                pos += 6;
            } else {
                out[pos++] = static_cast<char>(c);
            }
        }

        out[pos] = '\0';
    }
}
