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
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2
import QtQml.Models 2.2
import org.kde.kirigami 2.0 as Kirigami
import "toolbar"

Item {
    id: root

    Component.onCompleted: Context.debug("Context created")

    ColumnLayout {
        anchors.fill: parent

        ListView {
            id: appletListView

            signal scrollToApplet(string id)

            Layout.alignment: Qt.AlignTop
            Layout.fillHeight: true
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing
            displayMarginEnd: 100000
            displayMarginBeginning: 100000
            clip: true

            model: AppletProxyModel

            ScrollBar.vertical: ScrollBar { id: scrollBar }

            delegate: Loader {
                width: scrollBar.visible ? parent.width - scrollBar.width : parent.width
                active: true
                asynchronous: true

                function initialize() {
                    setSource(mainscript, {
                        "name": name,
                        "appletId": appletId,
                        "iconSource": "image://icon/" + icon,
                        "collapsed": collapsed,
                        "contentHeight": contentHeight,
                        "packagePath": packagePath,
                        "configEnabled": Qt.binding(function() { return appletToolbar.configEnabled; } )
                    });
                }

                Component.onCompleted: initialize()

                onStatusChanged: {
                    if (status == Loader.Error) {
                        Context.error("Error loading applet: " + appletId);
                        Context.error(sourceComponent.errorString());
                    }
                    if (status == Loader.Ready) {
                        Context.debug("Applet loaded: " + appletId);
                    }
                }

                Connections {
                    target: AppletProxyModel

                    onDataChanged: {
                        if (!!mainscript && mainscript != source) {
                            Context.debug("Data changed for applet " + appletId);
                            initialize();
                        }
                    }
                }
                Connections {
                    target: appletListView

                    onScrollToApplet: {
                        if (id == appletId) {
                            appletListView.positionViewAtIndex(index, ListView.Beginning);
                            Context.debug("Scroll to applet: " + appletId);
                        }
                    }
                }
            }
        }
        AppletToolbarAddItem {
            id: appletToolbarAddItem

            Layout.fillWidth: true
            height: Kirigami.Units.iconSizes.enormous
            visible: appletToolbar.configEnabled
        }
        AppletToolbar {
            id: appletToolbar

            contextRoot: root
            addItem: appletToolbarAddItem
            listView: appletListView
            Layout.alignment: Qt.AlignBottom
            Layout.fillWidth: true
        }
    }
}
