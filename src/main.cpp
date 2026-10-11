#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFontDatabase>
#include <QFont>
#include "controller/SessionController.h"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    app.setOrganizationName("DecisionHelper");
    app.setApplicationName("DecisionHelper");

    // ── Emoji font setup ──────────────────────────────────────────────────
    // Critical for WebAssembly which does not have access to host system fonts.
    // Loads Noto Color Emoji and sets it as fallback substitution for default font & Roboto.
#ifdef HAS_EMOJI_FONT
    const int fontId = QFontDatabase::addApplicationFont(
        QStringLiteral(":/fonts/NotoColorEmoji.ttf"));
    if (fontId == -1) {
        qWarning("Failed to load NotoColorEmoji.ttf from resources");
    } else {
        const QStringList families = QFontDatabase::applicationFontFamilies(fontId);
        if (!families.isEmpty()) {
            const QString emojiFamily = families.first();
            QFont::insertSubstitution(app.font().family(), emojiFamily);
            QFont::insertSubstitution(QStringLiteral("Roboto"), emojiFamily);
            QFont::insertSubstitution(QStringLiteral("sans-serif"), emojiFamily);
        }
    }
#endif

    SessionController controller;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("sessionController", &controller);
    const QUrl url(QStringLiteral("qrc:/qt/qml/DecisionHelper/src/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject* obj, const QUrl& objUrl) {
        if (!obj && url == objUrl) QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
