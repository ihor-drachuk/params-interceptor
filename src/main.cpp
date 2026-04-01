#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QProcessEnvironment>
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <QUuid>
#include <cstdio>

// Environment variables for configuration:
// PARAMS_INTERCEPTOR_LOGS_DIR      - Override logs directory
// PARAMS_INTERCEPTOR_LOGS_NAME     - Override logs filename
// PARAMS_INTERCEPTOR_LOG_PARAMS    - Set to "0" to disable logging params
// PARAMS_INTERCEPTOR_LOG_WORKDIR   - Set to "0" to disable logging working dir
// PARAMS_INTERCEPTOR_LOG_ENV       - Set to "0" to disable logging env variables
// PARAMS_INTERCEPTOR_LOG_FILES     - Set to "1" to log content of files from params
// PARAMS_INTERCEPTOR_ADD_PARAMS    - Params to add (separated by |)
// PARAMS_INTERCEPTOR_REPLACE_PARAMS - Params to replace (format: old1=new1|old2=new2)
// PARAMS_INTERCEPTOR_REMOVE_PARAMS - Params to remove (separated by |)

bool isEnvEnabled(const QString& envName, bool defaultValue)
{
    auto value = QProcessEnvironment::systemEnvironment().value(envName);
    if (value.isEmpty())
        return defaultValue;
    return value != "0" && value.toLower() != "false";
}

QStringList parseEnvList(const QString& envName)
{
    auto value = QProcessEnvironment::systemEnvironment().value(envName);
    if (value.isEmpty())
        return QStringList();
    return value.split('|', Qt::SkipEmptyParts);
}

QMap<QString, QString> parseEnvMap(const QString& envName)
{
    QMap<QString, QString> result;
    auto value = QProcessEnvironment::systemEnvironment().value(envName);
    if (value.isEmpty())
        return result;

    auto pairs = value.split('|', Qt::SkipEmptyParts);
    for (const auto& pair : pairs) {
        int idx = pair.indexOf('=');
        if (idx > 0) {
            result[pair.left(idx)] = pair.mid(idx + 1);
        }
    }
    return result;
}

void logFileContent(QTextStream& stream, const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile())
        return;

    // Limit file size to 1MB to prevent huge logs
    if (fileInfo.size() > 1024 * 1024) {
        stream << QString("\n--- File content of \"%1\" (truncated, file too large: %2 bytes) ---\n")
                      .arg(filePath).arg(fileInfo.size());
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        stream << QString("\n--- Cannot read file \"%1\" ---\n").arg(filePath);
        return;
    }

    stream << QString("\n--- File content of \"%1\" ---\n").arg(filePath);
    QTextStream fileStream(&file);
    stream << fileStream.readAll();
    stream << QString("\n--- End of file \"%1\" ---\n").arg(filePath);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Use QCoreApplication::arguments() for proper Unicode support
    QStringList args = QCoreApplication::arguments();

    auto currentDir = QDir::currentPath();
    auto targetName = QFileInfo(args.at(0)).fileName();

    // Configuration from environment
    auto overrideLogsDir = QProcessEnvironment::systemEnvironment().value("PARAMS_INTERCEPTOR_LOGS_DIR");
    auto overrideLogsName = QProcessEnvironment::systemEnvironment().value("PARAMS_INTERCEPTOR_LOGS_NAME");
    auto defaultLogsDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);

    // Better handling of multiple instances - add unique ID to filename
    auto uniqueId = QUuid::createUuid().toString(QUuid::Id128).left(8);
    auto timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    auto defaultLogsName = QString("interceptor-%1-%2-%3.txt").arg(targetName, timestamp, uniqueId);

    auto logsName = overrideLogsName.size() ? overrideLogsName : defaultLogsName;
    auto logsDir =  overrideLogsDir.size() ? overrideLogsDir : defaultLogsDir;

    auto logsPath = QFileInfo(logsDir, logsName).absoluteFilePath();

    QFile file(logsPath);
    file.open(QIODevice::WriteOnly | QIODevice::Append);

    QTextStream stream(&file);
    // Qt6 uses UTF-8 by default for QTextStream

    // Logging configuration
    bool logParams = isEnvEnabled("PARAMS_INTERCEPTOR_LOG_PARAMS", true);
    bool logWorkDir = isEnvEnabled("PARAMS_INTERCEPTOR_LOG_WORKDIR", true);
    bool logEnv = isEnvEnabled("PARAMS_INTERCEPTOR_LOG_ENV", true);
    bool logFiles = isEnvEnabled("PARAMS_INTERCEPTOR_LOG_FILES", false);

    // Params modification configuration
    QStringList addParams = parseEnvList("PARAMS_INTERCEPTOR_ADD_PARAMS");
    QMap<QString, QString> replaceParams = parseEnvMap("PARAMS_INTERCEPTOR_REPLACE_PARAMS");
    QStringList removeParams = parseEnvList("PARAMS_INTERCEPTOR_REMOVE_PARAMS");

    stream << QString("\n");
    stream << QString("--------------------------------------------------\n");
    stream << QDateTime::currentDateTime().toString() << "\n";
    stream << QString("\n");

    // Log command line with proper escaping
    stream << "\"" << QFileInfo(args.at(0)).absoluteFilePath().replace("/", "\\") << "\"";
    for (int i = 1; i < args.size(); i++)
        stream << " \"" << QString(args.at(i)).replace("\"", "\\\"") << "\"";
    stream << QString("\n");

    if (logWorkDir) {
        stream << QString("\n");
        stream << QString("Working directory: %1\n").arg(currentDir).replace("/", "\\");
    }

    if (logParams) {
        stream << QString("\n");
        stream << QString("Params count: %1\n").arg(args.size());

        for (int i = 0; i < args.size(); i++)
            stream << QString("   Param #%1: %2\n").arg(i).arg(args.at(i));
    }

    if (logEnv) {
        stream << "\n";
        stream << "Environment variables:\n";
        auto envs = QProcessEnvironment::systemEnvironment().toStringList();
        for (const auto& x : std::as_const(envs))
            stream << "    " << x << "\n";
    }

    // Log content of files specified in parameters
    if (logFiles) {
        stream << "\n";
        stream << "=== Files content from parameters ===\n";
        for (int i = 1; i < args.size(); i++) {
            QString param = args.at(i);
            QFileInfo fi(param);
            if (fi.exists() && fi.isFile()) {
                logFileContent(stream, fi.absoluteFilePath());
            }
        }
    }

    // Prepare parameters for the target process
    QStringList params;
    for (int i = 1; i < args.size(); i++) {
        QString param = args.at(i);

        // Check for removal
        bool shouldRemove = false;
        for (const auto& toRemove : removeParams) {
            if (param == toRemove || param.startsWith(toRemove + "=")) {
                shouldRemove = true;
                break;
            }
        }
        if (shouldRemove)
            continue;

        // Check for replacement
        bool replaced = false;
        for (auto it = replaceParams.constBegin(); it != replaceParams.constEnd(); ++it) {
            if (param == it.key()) {
                params.append(it.value());
                replaced = true;
                break;
            }
        }
        if (!replaced) {
            params.append(param);
        }
    }

    // Add additional parameters
    params.append(addParams);

    // Log modifications if any were made
    if (!addParams.isEmpty() || !replaceParams.isEmpty() || !removeParams.isEmpty()) {
        stream << "\n";
        stream << "=== Parameter modifications ===\n";
        if (!removeParams.isEmpty())
            stream << "Removed: " << removeParams.join(", ") << "\n";
        if (!replaceParams.isEmpty()) {
            stream << "Replaced: ";
            QStringList replaceList;
            for (auto it = replaceParams.constBegin(); it != replaceParams.constEnd(); ++it)
                replaceList << QString("%1 -> %2").arg(it.key(), it.value());
            stream << replaceList.join(", ") << "\n";
        }
        if (!addParams.isEmpty())
            stream << "Added: " << addParams.join(", ") << "\n";
        stream << "Final params: ";
        for (const auto& p : params)
            stream << "\"" << p << "\" ";
        stream << "\n";
    }

    QString program = args.at(0);
    program.append("_orig");

    QProcess proc;
    proc.setProcessChannelMode(QProcess::ForwardedChannels);
    proc.setInputChannelMode(QProcess::ForwardedInputChannel);
    proc.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    proc.setWorkingDirectory(currentDir);

    proc.start(program, params);

    if (!proc.waitForStarted(2000)) {
        fprintf(stderr, "Failed to start target process!\n");
        fflush(stderr);
        return -1;
    }

    if (proc.state() == QProcess::ProcessState::Running) {
        proc.waitForFinished(-1);
    }

    app.processEvents();

    return proc.exitCode();
}
