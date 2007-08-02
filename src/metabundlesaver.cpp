// Jeff Mitchell <kde-dev@emailgoeshere.com>, (C) 2006
// License: GNU General Public License V2


#define DEBUG_PREFIX "MetaBundleSaver"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>

#include "amarok.h"
#include "amarokconfig.h"
#include "collectiondb.h"
#include "debug.h"
#include "metabundlesaver.h"
#include "scancontroller.h"
#include <kapplication.h>
#include <kfilemetainfo.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kio/netaccess.h>
#include <kcodecs.h>
#include <kurl.h>
#include <QFile> //decodePath()
#include <QByteArray>
#include <QString>
#include <fileref.h>

#include "config-amarok.h"

#include "metabundle.h"

MetaBundleSaver::MetaBundleSaver( MetaBundle *bundle )
    : QObject()
    , m_bundle( bundle )
    , m_tempSavePath( QString() )
    , m_origRenamedSavePath( QString() )
    , m_tempSaveDigest( 0 )
    , m_saveFileref( 0 )
    , m_maxlen( 8192 )
    , m_cleanupNeeded( false )

{
    DEBUG_BLOCK
}

MetaBundleSaver::~MetaBundleSaver()
{
    DEBUG_BLOCK
    if( m_cleanupNeeded )
        cleanupSave();
}

TagLib::FileRef *
MetaBundleSaver::prepareToSave()
{
    DEBUG_BLOCK

    m_cleanupNeeded = true;
    KMD5 md5sum( 0, 0 );
    const KUrl origPath = m_bundle->url();
    char hostbuf[32];
    int hostname = gethostname( hostbuf, 32 );
    hostbuf[31] = '\0';
    if( hostname != 0 )
    {
        debug() << "Could not determine hostname!";
        return 0;
    }
    QString pid;
    QString randomString = m_bundle->getRandomString( 8, true );
    m_tempSavePath = origPath.path() + ".amaroktemp.host-" + QString( hostbuf ) +
                        ".pid-" + pid.setNum( getpid() ) + ".random-" + randomString + '.' + m_bundle->type();
    m_origRenamedSavePath = origPath.path() + ".amarokoriginal.host-" + QString( hostbuf ) +
                        ".pid-" + pid.setNum( getpid() ) + ".random-" + randomString + '.' + m_bundle->type();


    //The next long step is to copy the file over.  We can't use KIO because it's not thread save,
    //and std and QFile only have provisions for renaming and removing, so manual it is
    //doing it block-by-block results it not needing a huge amount of memory overhead

    debug() << "Copying original file to copy and caluclating MD5";

    if( QFile::exists( m_tempSavePath ) )
    {
        debug() << "Temp file already exists!";
        return 0;
    }

    QFile orig( m_bundle->url().path() );
    QFile copy( m_tempSavePath );

    if( !orig.open( QIODevice::Unbuffered | QIODevice::ReadOnly ) )
    {
        debug() << "Could not open original file!";
        return 0;
    }

    //Do this separately so as not to create a zero-length file if you can't read from input
    if( !copy.open( QIODevice::Unbuffered | QIODevice::WriteOnly | QIODevice::Truncate ) )
    {
        debug() << "Could not create file copy";
        return 0;
    }

    Q_LONG actualreadlen, actualwritelen;

    while( ( actualreadlen = orig.read( m_databuf, m_maxlen ) ) > 0 )
    {
        md5sum.update( m_databuf, actualreadlen );
        if( ( actualwritelen = copy.write( m_databuf, actualreadlen ) ) != actualreadlen )
        {
            debug() << "Error during copying of original file data to copy!";
            return 0;
        }
    }

    if( actualreadlen == -1 )
    {
        debug() << "Error during reading original file!";
        return 0;
    }

    m_tempSaveDigest = md5sum.hexDigest();

    //By this point, we have the following:
    //The original file is copied at path m_tempSavePath
    //We have generated what will be the filename to rename the original to in m_origRenamedSavePath
    //We have successfully copied the original file to the temp location
    //We've calculated the md5sum of the original file

    debug() << "MD5 sum of temp file: " << m_tempSaveDigest.data();

    //Now, we have a MD5 sum of the original file at the time of copying saved in m_tempSaveDigest
    //Create a fileref on the copied file, for modification

    m_saveFileref = new TagLib::FileRef( QFile::encodeName( m_tempSavePath ), false );

    if( m_saveFileref && !m_saveFileref->isNull() )
        return m_saveFileref;

    debug() << "Error creating temp file's fileref!";
    return 0;
}

bool
MetaBundleSaver::doSave()
{
    //TODO: much commenting needed.  For now this pretty much follows algorithm laid out in bug 131353,
    //but isn't useable since I need to find a good way to switch the file path with taglib, or a good way
    //to get all the metadata copied over.

    DEBUG_BLOCK
    m_cleanupNeeded = true;
    bool revert = false;

    QFile origRenamedFile( m_origRenamedSavePath );
    KMD5 md5sum( 0, 0 );
    Q_LONG actualreadlen;

    int errcode;

    QByteArray origRenamedDigest;

    if( !m_saveFileref || m_tempSavePath.isEmpty() || m_tempSaveDigest.isEmpty() || m_origRenamedSavePath.isEmpty() )
    {
        debug() << "You must run prepareToSave() and it must return successfully before calling doSave()!";
        return false;
    }

    debug() << "Saving tag changes to the temporary file...";

    //We've made our changes to the fileref; save it first, then do the logic to move the correct file back
    if( !m_saveFileref->save() )
    {
        debug() << "Could not save the new file!";
        goto fail_remove_copy;
    }

    debug() << "Renaming original file to temporary name " << m_origRenamedSavePath;

    errcode = std::rename( QFile::encodeName( m_bundle->url().path() ).data(),
                               QFile::encodeName( m_origRenamedSavePath ).data() );
    if( errcode != 0 )
    {
        debug() << "Could not move original!";
        perror( "Could not move original!" );
        goto fail_remove_copy;
    }

    revert = true;

    debug() << "Calculating MD5 of " << m_origRenamedSavePath;

    if( !origRenamedFile.open( QIODevice::Unbuffered | QIODevice::ReadOnly ) )
    {
        debug() << "Could not open temporary file!";
        goto fail_remove_copy;
    }

    while( ( actualreadlen = origRenamedFile.read( m_databuf, m_maxlen ) ) > 0 )
        md5sum.update( m_databuf, actualreadlen );

    if( actualreadlen == -1 )
    {
        debug() << "Error during checksumming temp file!";
        goto fail_remove_copy;
    }

    origRenamedDigest = md5sum.hexDigest();

    debug() << "md5sum of original file: " << origRenamedDigest.data();

    if( origRenamedDigest != m_tempSaveDigest )
    {
        debug() << "Original checksum did not match current checksum!";
        goto fail_remove_copy;
    }

    debug() << "Renaming temp file to original's filename";

    errcode = std::rename( QFile::encodeName( m_tempSavePath ).data(),
                                QFile::encodeName( m_bundle->url().path() ).data() );
    if( errcode != 0 )
    {
        debug() << "Could not rename newly-tagged file to original!";
        perror( "Could not rename newly-tagged file to original!" );
        goto fail_remove_copy;
    }

    debug() << "Deleting original";

    errcode = std::remove( QFile::encodeName( m_origRenamedSavePath ) );
    if( errcode != 0 )
    {
        debug() << "Could not delete the original file!";
        perror( "Could not delete the original file!" );
        return false;
    }

    debug() << "Save done, returning true!";

    return true;

    fail_remove_copy:

        debug() << "Deleting temporary file...";
        errcode = std::remove( QFile::encodeName( m_tempSavePath ).data() );
        if( errcode != 0 )
        {
            debug() << "Could not delete the temporary file!";
            perror( "Could not delete the temporary file!" );
        }

        if( !revert )
            return false;

        debug() << "Reverting original file to original filename!";
        errcode = std::rename( QFile::encodeName( m_origRenamedSavePath ).data(),
                                QFile::encodeName( m_bundle->url().path() ).data() );
        if( errcode != 0 )
        {
            debug() << "Could not revert file to original filename!";
            perror( "Could not revert file to original filename!" );
        }

        return false;
}

bool
MetaBundleSaver::cleanupSave()
{
    DEBUG_BLOCK

    bool dirty = false;

    if( !m_tempSavePath.isEmpty() && QFile::exists( m_tempSavePath ) )
    {
        int errcode;
        errcode = std::remove( QFile::encodeName( m_tempSavePath ).data() );
        if( errcode != 0 )
        {
            dirty = true;
            debug() << "Could not delete the temporary file!";
        }
    }

    m_tempSavePath.clear();
    m_origRenamedSavePath.clear();
    m_tempSaveDigest = QByteArray( 0 );
    if( m_saveFileref )
    {
        delete m_saveFileref;
        m_saveFileref = 0;
    }

    m_cleanupNeeded = false;
    return !dirty;
}

#include "metabundlesaver.moc"
