/****************************************************************************************
 * Copyright (c) 2013 Konrad Zemek <konrad.zemek@gmail.com>                             *
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

#ifndef STATSYNCING_ITUNES_PROVIDER_H
#define STATSYNCING_ITUNES_PROVIDER_H

#include "importers/ImporterProvider.h"

namespace StatSyncing
{

class ITunesProvider : public ImporterProvider
{
    Q_OBJECT

public:
    ITunesProvider( const QVariantMap &config, ImporterManager *importer );
    ~ITunesProvider();

    qint64 reliableTrackMetaData() const;
    qint64 writableTrackStatsData() const;

    // Methods called only from non-main thread
    QSet<QString> artists();
    TrackList artistTracks( const QString &artistName );
};

} // namespace StatSyncing

#endif // STATSYNCING_ITUNES_PROVIDER_H
