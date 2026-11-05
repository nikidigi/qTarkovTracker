#ifndef SCREENSHOTPARSER_H
#define SCREENSHOTPARSER_H

#include <QString>
#include <QRegularExpression>
#include <QFileInfo>
#include <QtMath>

#include <optional>
#include <cmath>

struct ScreenshotPosition
{
    double x;
    double y;
    double z;
    double direction;
};

class ScreenshotParser
{
public:
    static std::optional<ScreenshotPosition> parse(const QString &filename)
    {
        static const QRegularExpression regex(
                R"(_(-?\d+\.\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+)_(-?\d+\.\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+)_(-?\d+\.\d+))");

        const auto match = regex.match(QFileInfo(filename).fileName());

        if (!match.hasMatch())
            return std::nullopt;

        const double x = match.captured(1).toDouble();
        const double y = match.captured(2).toDouble();
        const double z = match.captured(3).toDouble();

        const double qx = match.captured(4).toDouble();
        const double qy = match.captured(5).toDouble();
        const double qz = match.captured(6).toDouble();
        const double qw = match.captured(7).toDouble();

        return ScreenshotPosition{ x, y, z, getYawFromQuaternion(qx, qy, qz, qw) };
    }

private:
    static double getYawFromQuaternion(double x, double y, double z, double w)
    {
        const double sinyCosp = 2.0 * (w * y + x * z);
        const double cosyCosp = 1.0 - 2.0 * (y * y + z * z);

        return qRadiansToDegrees(std::atan2(sinyCosp, cosyCosp));
    }
};

#endif // SCREENSHOTPARSER_H
