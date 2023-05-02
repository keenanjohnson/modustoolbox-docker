/*
 * Copyright 2018-2022 Cypress Semiconductor Corporation (an Infineon company)
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <QDir>
#include <QString>
#include <QXmlStreamReader>

namespace cydfuht {
namespace core {

/** The separator in string representations of versions. */
static const QLatin1String VERSION_SEPARATOR = QLatin1String(".");
/** The file name used for version files. */
static const QLatin1String VERSION_FILENAME = QLatin1String("version.xml");

/** Represents a version with major, minor, patch, and build components */
class CyVersion {
   public:
    /** The major version number */
    int Major;
    /** The minor version number */
    int Minor;
    /** The patch version number, or UNSPECIFIED_ELEMENT if not specified */
    int Patch;
    /** The build number, or UNSPECIFIED_ELEMENT if not specified */
    int Build;

    /**
     * Constructs a new CyVersion.
     *
     * @param major the major version
     * @param minor the minor version
     * @param patch the patch version (optional)
     * @param build the build version (optional)
     */
    explicit CyVersion(int major, int minor = UNSPECIFIED_ELEMENT, int patch = UNSPECIFIED_ELEMENT, int build = UNSPECIFIED_ELEMENT)
        : Major(major), Minor(minor), Patch(patch), Build(build) {}

    /**
     * Constructs a blank version.
     */
    CyVersion() : CyVersion(0, 0, 0, 0) {}

    /**
     * Parses a version identifier from the specified string.
     * See <code>CyVersion(String)</code> for the format of the version string.
     *
     * @param version String representation of the version identifier. Leading and trailing whitespace will be ignored.
     * @return A <code>CyVersion</code> object representing the version identifier.
     */
    static CyVersion create(const QString &version) {
        static const int MAJOR_IDX = 0;
        static const int MINOR_IDX = 1;
        static const int PATCH_IDX = 2;
        static const int BUILD_IDX = 3;

        QString trimmed = version.trimmed();
        if (trimmed.length() == 0) {
            return CyVersion();
        }

        int major = UNSPECIFIED_ELEMENT;
        int minor = UNSPECIFIED_ELEMENT;
        int build = UNSPECIFIED_ELEMENT;
        int patch = UNSPECIFIED_ELEMENT;

        QStringList list = trimmed.split(VERSION_SEPARATOR);
        bool ok = (list.size() > MAJOR_IDX && list.size() <= (BUILD_IDX + 1));
        if (ok) major = list[MAJOR_IDX].toInt(&ok);
        if (ok && list.size() > MINOR_IDX)
            minor = list[MINOR_IDX].toInt(&ok);
        else
            minor = 0;  // Versions always have a minor, it is implicitly zero if missing
        if (ok && list.size() > PATCH_IDX) patch = list[PATCH_IDX].toInt(&ok);
        if (ok && list.size() > BUILD_IDX) build = list[BUILD_IDX].toInt(&ok);

        return ok ? CyVersion(major, minor, patch, build) : CyVersion();
    }

    /**
     * Load a version identifier from the specified file.
     *
     * @param path Location of xml file with string representation of the version identifier.
     * @return A <code>CyVersion</code> object representing the version identifier.
     */
    static CyVersion loadFromVersionFile(const QString &path) {
        QString parsedPath = path;
#if defined(Q_OS_LINUX) || defined(Q_OS_DARWIN)
        QStringList folders = parsedPath.split(QDir::separator());
        if (folders.size() > 1)  // need to remove last folder specific for Mac OS an Linux to get to version.xml
        {
            folders.removeLast();  // remove MacOS for Mac OS or bin for Linux
#ifdef Q_OS_DARWIN
            folders.append(QLatin1String("Resources"));
#endif
            parsedPath = QDir::cleanPath(folders.join(QDir::separator()));
        }
#endif
        parsedPath = QDir::cleanPath(parsedPath + QDir::separator() + VERSION_FILENAME);
        QString version(QLatin1String("-1.-1.-1.-1"));
        if (QFileInfo::exists(parsedPath)) {
            QFile file(parsedPath);
            if (!file.exists()) return CyVersion();
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return CyVersion();
            QTextStream stream(&file);
            stream.setCodec("UTF-8");
            QString xmlString = stream.readAll();
            QXmlStreamReader reader(xmlString);

            QStringRef temp = reader.name();
            if (reader.readNextStartElement()) {
                if (reader.name() == QLatin1String("version")) {
                    version = reader.readElementText();
                }
            }
        }

        return CyVersion::create(version);
    }

   private:
    /** The number that indicates that a minor/patch/build number was not specified. */
    static const int UNSPECIFIED_ELEMENT = -1;
};

}  // namespace core
}  // namespace cydfuht
