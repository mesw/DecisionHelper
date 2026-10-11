#include "Storage.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

void Storage::save(const std::string& key, const std::string& value) {
    EM_ASM({
        var k = UTF8ToString($0);
        var v = UTF8ToString($1);
        localStorage.setItem(k, v);
    }, key.c_str(), value.c_str());
}

std::string Storage::load(const std::string& key) {
    char* raw = (char*)EM_ASM_PTR({
        var k = UTF8ToString($0);
        var v = localStorage.getItem(k);
        if (v === null) return null;
        var len = lengthBytesUTF8(v) + 1;
        var buf = _malloc(len);
        stringToUTF8(v, buf, len);
        return buf;
    }, key.c_str());
    if (!raw) return {};
    std::string result(raw);
    free(raw);
    return result;
}

bool Storage::exists(const std::string& key) {
    int found = EM_ASM_INT({
        return localStorage.getItem(UTF8ToString($0)) !== null ? 1 : 0;
    }, key.c_str());
    return found != 0;
}

void Storage::remove(const std::string& key) {
    EM_ASM({
        localStorage.removeItem(UTF8ToString($0));
    }, key.c_str());
}

#else
// Desktop fallback: use QFile
#include <QFile>
#include <QDir>
#include <QStandardPaths>

static QString storagePath(const std::string& key) {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/" + QString::fromStdString(key) + ".json";
}

void Storage::save(const std::string& key, const std::string& value) {
    QFile f(storagePath(key));
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        f.write(QByteArray::fromStdString(value));
}

std::string Storage::load(const std::string& key) {
    QFile f(storagePath(key));
    if (!f.open(QIODevice::ReadOnly)) return {};
    return f.readAll().toStdString();
}

bool Storage::exists(const std::string& key) {
    return QFile::exists(storagePath(key));
}

void Storage::remove(const std::string& key) {
    QFile::remove(storagePath(key));
}
#endif
