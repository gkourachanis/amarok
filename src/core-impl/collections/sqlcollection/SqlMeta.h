/****************************************************************************************
 * Copyright (c) 2007 Maximilian Kossick <maximilian.kossick@googlemail.com>            *
 * Copyright (c) 2007 Alexandre Pereira de Oliveira <aleprj@gmail.com>                  *
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

#ifndef SQLMETA_H
#define SQLMETA_H

#include "core/meta/Meta.h"
#include "amarok_sqlcollection_export.h"

#include <QByteArray>
#include <QDateTime>
#include <QHash>
#include <QMap>
#include <QMutex>
#include <QString>
#include <QStringList>
#include <QVariant>

namespace Capabilities {
    class AlbumCapabilityDelegate;
    class ArtistCapabilityDelegate;
    class TrackCapabilityDelegate;
}
class QAction;

namespace Collections {
    class SqlCollection;
}

namespace Meta
{

class SqlTrack : public Meta::Track
{
    public:
        /** returns a string of all database values that can be fetched for a track */
        static QString getTrackReturnValues();
        /** returns the number of return values in getTrackReturnValues() */
        static int getTrackReturnValueCount();
        static TrackPtr getTrack( int deviceid, const QString &rpath, Collections::SqlCollection *collection );
        static TrackPtr getTrackFromUid( const QString &uid, Collections::SqlCollection *collection );

        SqlTrack( Collections::SqlCollection *collection, const QStringList &queryResult );
        ~ SqlTrack();

        /** returns the title of this track as stored in the database **/
        virtual QString name() const { return m_title; }
        /** returns the title of the track if existing in the database,
            a value deduced from the file name otherwise */
        virtual QString prettyName() const;
        /** returns "[artist] - [title]" if both are stored in the database,
            a value deduced from the file name otherwise */
        virtual QString fullPrettyName() const;

        /** returns the KUrl object describing the position of the track */
        virtual KUrl playableUrl() const { return m_url; }
        /** returns a string describing the position of the track; same as url() */
        virtual QString prettyUrl() const { return m_url.path(); }
        /** returns a string describing the position of the track */
        virtual QString uidUrl() const { return m_uid; }

        /** true if there is a collection and the file exists on disk */
        virtual bool isPlayable() const;
        /** true if there is a collection, the file exists on disk and is writeable */
        virtual bool isEditable() const;

        virtual Meta::AlbumPtr album() const { return m_album; }
        virtual void setAlbum( const QString &newAlbum );
        virtual void setArtist( const QString &newArtist );
        virtual Meta::ArtistPtr artist() const { return m_artist; }
        virtual Meta::ComposerPtr composer() const { return m_composer; }
        virtual void setComposer( const QString &newComposer );
        virtual Meta::YearPtr year() const { return m_year; }
        virtual void setYear( const QString &newYear );
        virtual Meta::GenrePtr genre() const { return m_genre; }
        virtual void setGenre( const QString &newGenre );

        virtual QString type() const;

        //helper functions
        static QString prettyTitle( const QString &filename );

        virtual void setTitle( const QString &newTitle );

        virtual void setUrl( const QString &url );
        virtual void setUrl( const int deviceid, const QString &rpath );

        virtual qreal bpm() const { return m_bpm; }
        virtual void setBpm( const qreal newBpm );

        virtual QString comment() const { return m_comment; }
        virtual void setComment( const QString &newComment );

        virtual double score() const { return m_score; }
        virtual void setScore( double newScore );

        virtual int rating() const { return m_rating; }
        virtual void setRating( int newRating );

        virtual qint64 length() const { return m_length; }
        virtual int filesize() const { return m_filesize; }
        virtual int sampleRate() const { return m_sampleRate; }
        virtual int bitrate() const { return m_bitrate; }
        virtual QDateTime createDate() const { return m_createDate; }

        virtual int trackNumber() const { return m_trackNumber; }
        virtual void setTrackNumber( int newTrackNumber );

        virtual int discNumber() const { return m_discNumber; }
        virtual void setDiscNumber( int newDiscNumber );

        virtual uint firstPlayed() const { return m_firstPlayed; }
        virtual void setFirstPlayed( const uint newTime );

        virtual uint lastPlayed() const { return m_lastPlayed; }
        virtual void setLastPlayed( const uint newTime );

        virtual int playCount() const { return m_playCount; }
        virtual void setPlayCount( const int newCount );

        virtual qreal replayGain( ReplayGainMode mode ) const
        { return ( mode == AlbumReplayGain ) ? m_albumGain : m_trackGain; }
        virtual qreal replayPeakGain( ReplayGainMode mode ) const
        { return ( mode == AlbumReplayGain ) ? m_albumPeakGain : m_trackPeakGain; }

        virtual void setUidUrl( const QString &uid );

        virtual void beginMetaDataUpdate();
        virtual void endMetaDataUpdate();
        virtual void abortMetaDataUpdate();
        virtual void setWriteAllStatisticsFields( const bool enable ) { m_writeAllStatisticsFields = enable; }

        virtual void finishedPlaying( double playedFraction );

        virtual bool inCollection() const;
        virtual Collections::Collection* collection() const;

        virtual QString cachedLyrics() const;
        virtual void setCachedLyrics( const QString &lyrics );

        virtual bool hasCapabilityInterface( Capabilities::Capability::Type type ) const;

        virtual Capabilities::Capability* createCapabilityInterface( Capabilities::Capability::Type type );

        virtual void addLabel( const QString &label );
        virtual void addLabel( const Meta::LabelPtr &label );
        virtual void removeLabel( const Meta::LabelPtr &label );
        virtual Meta::LabelList labels() const;

        //SqlTrack specific methods
        int deviceid() const { return m_deviceid; }
        QString rpath() const { return m_rpath; }
        int trackId() const { return m_trackId; }
        Collections::SqlCollection* sqlCollection() const { return m_collection; }
        AMAROK_SQLCOLLECTION_EXPORT_TESTS void refreshFromDatabase( const QString &uid, Collections::SqlCollection* collection, bool updateObservers = true );
        void updateData( const QStringList &result, bool forceUpdates = false );
        void setCapabilityDelegate( Capabilities::TrackCapabilityDelegate *delegate );

    protected:
        void commitMetaDataChanges();
        void writeMetaDataToFile();
        void writeMetaDataToDb( const QStringList &fields );
        void writeMetaDataToDb( const QString &field ) { writeMetaDataToDb( QStringList( field ) ); }
        void updateStatisticsInDb( const QStringList &fields );
        void updateStatisticsInDb( const QString &field ) { updateStatisticsInDb( QStringList( field ) ); }

    private:
        /** returns a string of all database joins that are required to fetch all values for a track*/
        static QString getTrackJoinConditions();
        void updateFileSize();

        Collections::SqlCollection* m_collection;
        Capabilities::TrackCapabilityDelegate *m_capabilityDelegate;

        QString m_title;
        KUrl m_url;

        int m_deviceid;
        QString m_rpath;
        int m_trackId;

        qint64 m_length;
        qint64 m_filesize;
        int m_trackNumber;
        int m_discNumber;
        uint m_lastPlayed;
        uint m_firstPlayed;
        int m_playCount;
        int m_bitrate;
        int m_sampleRate;
        int m_rating;
        double m_score;
        QString m_comment;
        qreal m_bpm;
        QString m_uid;
        qreal m_albumGain;
        qreal m_albumPeakGain;
        qreal m_trackGain;
        qreal m_trackPeakGain;
        QDateTime m_createDate;

        Meta::AlbumPtr m_album;
        Meta::ArtistPtr m_artist;
        Meta::GenrePtr m_genre;
        Meta::ComposerPtr m_composer;
        Meta::YearPtr m_year;

        bool m_batchUpdate;
        bool m_writeAllStatisticsFields;
        QVariantMap m_cache;

        mutable bool m_labelsInCache;
        mutable Meta::LabelList m_labelsCache;

        QString m_newUid;
};

class SqlArtist : public Meta::Artist
{
    public:
        SqlArtist( Collections::SqlCollection* collection, int id, const QString &name );
        ~SqlArtist();

        virtual QString name() const { return m_name; }
        virtual QString prettyName() const { return m_name; } //change if necessary

        void updateData( Collections::SqlCollection* collection, int id, const QString &name );

        virtual void invalidateCache();

        virtual Meta::TrackList tracks();

        virtual Meta::AlbumList albums();

        virtual bool hasCapabilityInterface( Capabilities::Capability::Type type ) const;

        virtual Capabilities::Capability* createCapabilityInterface( Capabilities::Capability::Type type );

        //SQL specific methods
        int id() const { return m_id; }
        void setCapabilityDelegate( Capabilities::ArtistCapabilityDelegate *delegate ) { m_delegate = delegate; }


    private:
        Collections::SqlCollection* m_collection;
        Capabilities::ArtistCapabilityDelegate *m_delegate;
        QString m_name;
        int m_id;
        bool m_tracksLoaded;
        Meta::TrackList m_tracks;
        bool m_albumsLoaded;
        Meta::AlbumList m_albums;
        //QReadWriteLock does not support lock upgrades :(
        //see http://www.trolltech.com/developer/task-tracker/index_html?method=entry&id=131880
        //switch to QReadWriteLock as soon as it does!
        QMutex m_mutex;

};

class SqlAlbum : public Meta::Album
{
    public:
        SqlAlbum( Collections::SqlCollection* collection, int id, const QString &name, int artist );
        ~SqlAlbum();

        virtual QString name() const { return m_name; }
        virtual QString prettyName() const { return m_name; }

        void updateData( Collections::SqlCollection* collection, int id, const QString &name, int artist );

        virtual void invalidateCache();

        virtual Meta::TrackList tracks();

        virtual bool isCompilation() const;

        virtual bool hasAlbumArtist() const;
        virtual Meta::ArtistPtr albumArtist() const;

        //updating album images is possible for local tracks, but let's ignore it for now
        virtual bool hasImage( int size = 1 ) const;
        virtual bool canUpdateImage() const { return true; }
        virtual QPixmap image( int size = 1 );
        virtual KUrl imageLocation( int size = 1 );
        virtual void setImage( const QPixmap &pixmap );
        virtual void removeImage();
        virtual void setSuppressImageAutoFetch( const bool suppress ) { m_suppressAutoFetch = suppress; }
        virtual bool suppressImageAutoFetch() const { return m_suppressAutoFetch; }

        virtual bool hasCapabilityInterface( Capabilities::Capability::Type type ) const;

        virtual Capabilities::Capability* createCapabilityInterface( Capabilities::Capability::Type type );

        //SQL specific methods
        int id() const { return m_id; }

        void setCompilation( bool compilation );
        void setCapabilityDelegate( Capabilities::AlbumCapabilityDelegate *delegate ) { m_delegate = delegate; }
        Collections::SqlCollection *sqlCollection() const { return m_collection; }

    private:
        QByteArray md5sum( const QString& artist, const QString& album, const QString& file ) const;
        QString createScaledImage( QString path, int size ) const;
        QString findCachedImage( int size ) const;
        QString findLargeCachedImage() const;
        QString findImage( int size );
        QString imageKey() const;
        void updateImage( const QString path ) const; // Updates the database to ensure the album has the correct path
        // Finds or creates a magic value in the database which tells Amarok not to auto fetch an image since it has been explicitly unset.
        int unsetImageId() const;

    private:
        Collections::SqlCollection* m_collection;
        Capabilities::AlbumCapabilityDelegate *m_delegate;
        QString m_name;
        int m_id;
        int m_artistId;
        mutable bool m_hasImage;
        mutable bool m_hasImageChecked;
        mutable int m_unsetImageId;
        static const QString AMAROK_UNSET_MAGIC;
        mutable QHash<int, QString> m_images; // Cache mapping size -> path. hash used for O(1) insertion and O(1) lookup
        bool m_tracksLoaded;
        bool m_suppressAutoFetch;
        Meta::ArtistPtr m_artist;
        Meta::TrackList m_tracks;
        //QReadWriteLock does not support lock upgrades :(
        //see http://www.trolltech.com/developer/task-tracker/index_html?method=entry&id=131880
        //switch to QReadWriteLock as soon as it does!
        QMutex m_mutex;

        //TODO: add album artist
};

class SqlComposer : public Meta::Composer
{
    public:
        SqlComposer( Collections::SqlCollection* collection, int id, const QString &name );

        virtual QString name() const { return m_name; }
        virtual QString prettyName() const { return m_name; }

        void updateData( Collections::SqlCollection* collection, int id, const QString &name );

        virtual void invalidateCache();

        virtual Meta::TrackList tracks();

        //SQL specific methods
        int id() const { return m_id; }

    private:
        Collections::SqlCollection* m_collection;
        QString m_name;
        int m_id;
        bool m_tracksLoaded;
        Meta::TrackList m_tracks;
        //QReadWriteLock does not support lock upgrades :(
        //see http://www.trolltech.com/developer/task-tracker/index_html?method=entry&id=131880
        //switch to QReadWriteLock as soon as it does!
        QMutex m_mutex;
};

class SqlGenre : public Meta::Genre
{
    public:
        SqlGenre( Collections::SqlCollection* collection, int id, const QString &name );

        virtual QString name() const { return m_name; }
        virtual QString prettyName() const { return m_name; }

        void updateData( Collections::SqlCollection* collection, int id, const QString &name );

        virtual void invalidateCache();

        virtual Meta::TrackList tracks();

        //SQL specific methods
        int id() const { return m_id; }

    private:
        Collections::SqlCollection* m_collection;
        QString m_name;
        int m_id;
        bool m_tracksLoaded;
        Meta::TrackList m_tracks;
        //QReadWriteLock does not support lock upgrades :(
        //see http://www.trolltech.com/developer/task-tracker/index_html?method=entry&id=131880
        //switch to QReadWriteLock as soon as it does!
        QMutex m_mutex;
};

class SqlYear : public Meta::Year
{
    public:
        SqlYear( Collections::SqlCollection* collection, int id, const QString &name );

        virtual QString name() const { return m_name; }
        virtual QString prettyName() const { return m_name; }

        void updateData( Collections::SqlCollection* collection, int id, const QString &name );

        virtual void invalidateCache();

        virtual Meta::TrackList tracks();

        //SQL specific methods
        int id() const { return m_id; }

    private:
        Collections::SqlCollection* m_collection;
        QString m_name;
        int m_id;
        bool m_tracksLoaded;
        Meta::TrackList m_tracks;
        //QReadWriteLock does not support lock upgrades :(
        //see http://www.trolltech.com/developer/task-tracker/index_html?method=entry&id=131880
        //switch to QReadWriteLock as soon as it does!
        QMutex m_mutex;
};

class SqlLabel : public Meta::Label
{
public:
    SqlLabel( Collections::SqlCollection *collection, int id, const QString &name );

    virtual QString name() const { return m_name; }
    virtual QString prettyName() const { return m_name; }

    virtual void invalidateCache();

    virtual Meta::TrackList tracks();

    //SQL specific methods
    int id() const { return m_id; }

private:
    Collections::SqlCollection *m_collection;
    QString m_name;
    int m_id;
    bool m_tracksLoaded;
    Meta::TrackList m_tracks;
    //QReadWriteLock does not support lock upgrades :(
    //see http://www.trolltech.com/developer/task-tracker/index_html?method=entry&id=131880
    //switch to QReadWriteLock as soon as it does!
    QMutex m_mutex;
};

}

#endif /* SQLMETA_H */