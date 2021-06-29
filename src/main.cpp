#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QProcessEnvironment>
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <cstdio>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    auto currentDir = QDir::currentPath();
    auto targetName = QFileInfo(argv[0]).fileName();

    auto overrideLogsDir = QProcessEnvironment::systemEnvironment().value("PARAMS_INTERCEPTOR_LOGS_DIR");
    auto overrideLogsName = QProcessEnvironment::systemEnvironment().value("PARAMS_INTERCEPTOR_LOGS_NAME");
    auto defaultLogsDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    auto defaultLogsName = "interceptor-" + targetName + ".txt";

    auto logsName = overrideLogsName.size() ? overrideLogsName : defaultLogsName;
    auto logsDir =  overrideLogsDir.size() ? overrideLogsDir : defaultLogsDir;

    auto logsPath = QFileInfo(logsDir, logsName).absoluteFilePath();

    QFile file(logsPath);
    file.open(QIODevice::WriteOnly | QIODevice::Append);

    QTextStream stream(&file);

    stream << QString("\n");
    stream << QString("--------------------------------------------------\n");
    stream << QDateTime::currentDateTime().toString() << "\n";
    stream << QString("\n");
    stream << "\"" << QFileInfo(targetName).absoluteFilePath().replace("/", "\\") << "\"";
    for (int i = 1; i < argc; i++)
        stream << " \"" << QString(argv[i]).replace("\"", "\\\"").toUtf8() << "\"";
    stream << QString("\n");

    stream << QString("\n");
    stream << QString("Working directory: %1\n").arg(currentDir).replace("/", "\\");
    stream << QString("\n");
    stream << QString("Params count: %1\n").arg(argc).toUtf8();

    for (int i = 0; i < argc; i++)
        stream << QString("   Param #%1: %2\n").arg(i).arg(argv[i]).toUtf8();

    stream << "\n";

    stream << "Environment variables:\n";
    auto envs = QProcessEnvironment::systemEnvironment().toStringList();
    for (const auto& x : qAsConst(envs))
        stream << "    " << x << "\n";

    QString program = argv[0];
    program.append("_orig");
    QStringList params;
    for (int i = 1; i < argc; i++)
        params.append(argv[i]);

    QProcess proc;
    proc.setProcessChannelMode(QProcess::ForwardedChannels);
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
