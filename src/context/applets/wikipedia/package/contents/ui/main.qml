/****************************************************************************************
 * Copyright (c) 2017 Malte Veerman <malte.veerman@gmail.com>                           *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtWebEngine 1.1
import org.kde.kirigami 2.0 as Kirigami
import org.kde.amarok.qml 1.0 as AmarokQml
import org.kde.amarok.wikipedia 1.0

AmarokQml.Applet {
    id: applet

    ColumnLayout {
        anchors.fill: parent

        RowLayout {
            Layout:fillWidth: true
            Layout.alignment: Qt.AlignTop

            Button {
                iconName: "go-previous"
                enabled: content.canGoBack
                Layout.alignment: Qt.AlignLeft
                ToolTip.text: i18n("Previous")

                onClicked: content.goBack()
            }
            Button {
                iconName: "go-next"
                enabled: content.canGoForward
                Layout.alignment: Qt.AlignLeft
                ToolTip.text: i18n("Next")

                onClicked: content.goForward()
            }
            Button {
                iconName: "view-refresh"
                enabled: !content.loading
                Layout.alignment: Qt.AlignLeft
                ToolTip.text: i18n("Refresh")

                onClicked: content.reload()
            }
            Item {
                Layout.fillWidth: true
            }
            Button {
                iconName: "filename-artist-amarok"
                Layout.alignment: Qt.AlignRight
                ToolTip.text: i18n("Artist")

                onClicked: WikipediaEngine.selection = WikipediaEngine.Artist
            }
            Button {
                iconName: "filename-composer-amarok"
                Layout.alignment: Qt.AlignRight
                ToolTip.text: i18n("Composer")

                onClicked: WikipediaEngine.selection = WikipediaEngine.Composer
            }
            Button {
                iconName: "filename-album-amarok"
                Layout.alignment: Qt.AlignRight
                ToolTip.text: i18n("Album")

                onClicked: WikipediaEngine.selection = WikipediaEngine.Album
            }
            Button {
                iconName: "filename-title-amarok"
                Layout.alignment: Qt.AlignRight
                ToolTip.text: i18n("Track")

                onClicked: WikipediaEngine.selection = WikipediaEngine.Track
            }
        }

        WebEngineView {
            id: content

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignBottom
            height: Kirigami.Units.largeSpacing * 25 //TODO: Find a more elegant solution to set the height

            onNavigationRequested: {
                if (request.navigationType == WebEngineNavigationRequest.LinkClickedNavigation) {
                    request.action = WebEngineNavigationRequest.IgnoreRequest;
                    WikipediaEngine.url = request.url;
                }
            }

            Connections {
                target: WikipediaEngine

                onPageChanged: content.loadHtml(WikipediaEngine.page, WikipediaEngine.url)
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: WikipediaEngine.busy
            }
        }
    }
}
