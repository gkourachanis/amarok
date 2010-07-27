/****************************************************************************************
 * Copyright (c) 2008 Seb Ruiz <ruiz@kde.org>                                           *
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

#include "TrackItem.h"
#include "AlbumsDefs.h"
#include "core/meta/support/MetaUtility.h"

#include <QFont>

TrackItem::TrackItem()
    : QStandardItem()
{
    setEditable( false );
}

void
TrackItem::setTrack( Meta::TrackPtr trackPtr )
{
    if( m_track )
        unsubscribeFrom( m_track );
    m_track = trackPtr;
    subscribeTo( m_track );

    metadataChanged( m_track );
}

void
TrackItem::metadataChanged( Meta::TrackPtr track )
{
    setData( track->prettyName(), TrackNameRole );
    setData( track->artist()->prettyName(), TrackArtistRole );
    setData( track->trackNumber(), TrackNumberRole );
    setData( track->length(), TrackLengthRole );
    setData( track->album()->isCompilation(), AlbumCompilationRole );
    setData( track->album()->tracks().count(), AlbumTrackCountRole );
}

void
TrackItem::italicise()
{
    QFont f = font();
    f.setItalic( true );
    setFont( f );
}

void
TrackItem::bold()
{
    QFont f = font();
    f.setBold( true );
    setFont( f );
}

int
TrackItem::type() const
{
    return TrackType;
}
