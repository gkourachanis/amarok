
//
// Author: Max Howell (C) Copyright 2003
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef FAST_TRANSLATE
#include "amarokfilelist.h"    //for sorting directories
#endif
#include "engine/enginebase.h" //isValidMedia()
#include "metabundle.h"
#include "playerapp.h"
#include "playlistitem.h"
#include "playlistloader.h"
#include "playlistwidget.h"    //we're tied to this class

#include <qapplication.h>  //postEvent()
#include <qtextstream.h>   //loadM3U(),loadPLS()
#include <qfile.h>         //~PlaylistLoader()

#include <kapplication.h>
#include <kurl.h>
#include <kdebug.h>
#include <ktempfile.h>     //makePlaylistItem()
#include <kio/netaccess.h> //makePlaylistItem()
#include <kfileitem.h>     //process()
#include <kcursor.h>       //makePlaylistItem()
#include <kmessagebox.h>   //makePlaylistItem()
#include <klocale.h>       //makePlaylistItem()

//file stat
#include <dirent.h>        //process()
#include <sys/stat.h>      //process()

//some GNU systems don't support big files for some reason
#ifndef __USE_LARGEFILE64 //see dirent.h
 #define DIRENT dirent
 #define SCANDIR scandir
 #define READDIR readdir
 #define STATSTRUCT stat
 #define LSTAT stat
#else
 #define DIRENT dirent64
 #define SCANDIR scandir64
 #define READDIR readdir64
 #define STATSTRUCT stat64
 #define LSTAT stat64
#endif

/*
 * For pls and m3u specifications see: http://forums.winamp.com/showthread.php?s=dbec47f3a05d10a3a77959f17926d39c&threadid=65772
 */

//URGENT
//TODO store threads in a stack that can be emptied on premature program exit, or use one thread and a KURL::List stack
//TODO don't delete m_first, it may already have been removed! either make it unremovable or do something more intelligent

//LESS IMPORTANT
//TODO add non-local directories as items with a [+] next to, you open them by clicking the plus!! --maybe not
//TODO display dialog/something else that lists unloadable media after thread is exited
//TODO stop blocking on netaccess::download()
//TODO recursion limits
//TODO rethink recursion options <mxcl> IMHO they suck big chunks, always do it recursively, why drop/add a
//     directory if you don't want its contents?
//     you can always open the dir and select the files. Stupid option and PITA to implement.
//     <markey> non-recursive adding should get replaced by "add all media files in current directory"
//TODO reimplement ask recursive in PlaylistWidget::insertMedia()


PlaylistLoader::PlaylistLoader( const KURL::List &ul, PlaylistWidget *lv, QListViewItem *lvi )
   : m_list( ul )
   , m_after( lvi )
   , m_first( 0 )
   , m_listView( lv )
{}


//this is a private ctor used by ::makePlaylistItem()
//it can only be used with placeholders
PlaylistLoader::PlaylistLoader( const KURL::List &ul, PlaylistItem *item )
   : m_list( ul )
   , m_after( item )
   , m_first( item )
   , m_listView( item->listView() )
{}


PlaylistLoader::~PlaylistLoader()
{
    //for GUI access only

    if( NULL != m_first )
    {
        kdDebug() << "Unlinking tmpfile: " << m_list.first().path() << endl;
        QFile::remove( m_list.first().path() );
        delete m_first; //FIXME deleting m_first is dangerous as user may have done it for us!
    }

    kdDebug() << "[PLSloader] Done!\n";
}


void PlaylistLoader::run()
{
       kdDebug() << "[PLSloader] Started..\n";

       m_recursionCount = -1;
       process( m_list );

       QApplication::postEvent( m_listView, new PlaylistLoader::DoneEvent( this ) );
}


void PlaylistLoader::process( const KURL::List &list, const bool validate )
{
   struct STATSTRUCT statbuf;
   ++m_recursionCount;

   for( KURL::List::ConstIterator it = list.begin(); it != list.end(); ++it )
   {
      QString path = (*it).path();

      if( validate && (*it).isLocalFile() )
      {
         if( LSTAT( path.local8Bit(), &statbuf ) != 0 ) continue;

         if( S_ISDIR( statbuf.st_mode ) )
         {
            //some options prevent recursion
            //FIXME depth check too
            if( list.count() > 1 && ( !options.recurse || ( !options.symlink && S_ISLNK( statbuf.st_mode ) ) ) ) continue;
#ifdef FAST_TRANSLATE
            translate( path );
#else
            AmarokFileList files( options.sortSpec );
            files.setAutoDelete( true );
            translate( path, files );
            files.sort();

            KURL::List urls;
            for( KFileItemListIterator it( files ); *it; ++it )
            {
                urls << (*it)->url();
            }
            process( urls, false ); //false will prevent stating for dir, etc.
#endif
            continue;
         }
      }

      if( int type = isPlaylist( path ) )
      {
         if ( !m_recursionCount )     //prevent processing playlist files in subdirs
         {
            if( !(*it).isLocalFile() )
            {
                //if the playlist is not local, we need to d/l it, and KIO doesn't work in QThreads. sigh
                //so this will organise the d/l to occur syncronously and then a new thread spawned :)
                postDownloadRequest( *it );
            }
            else
            {
                loadLocalPlaylist( path, type );
            }
         }
      }
      else
      {
         if( validate && !isValidMedia( *it ) ) continue; //TODO retain stat info if done above, which does happen

         postBundle( *it );
      }
   }
}


inline
void PlaylistLoader::postDownloadRequest( const KURL &u )
{
    QApplication::postEvent( m_listView, new PlaylistLoader::DownloadPlaylistEvent( this, u ) );
}

inline
void PlaylistLoader::postBundle( const KURL &u )
{
    QApplication::postEvent( m_listView, new PlaylistLoader::MakeItemEvent( this, u, QString::null, MetaBundle::Undetermined ) );
}

inline
void PlaylistLoader::postBundle( const KURL &u, const QString &s, const int i )
{
    QApplication::postEvent( m_listView, new PlaylistLoader::MakeItemEvent( this, u, s ,i ) );
}


int PlaylistLoader::isPlaylist( const QString &path ) //static
{
   //TODO investigate faster methods
   //TODO try to achieve retVal optimisation

        if( path.endsWith( ".m3u", false ) ) return 1;
   else if( path.endsWith( ".pls", false ) ) return 2;
   else return 0;
}


void PlaylistLoader::loadLocalPlaylist( const QString &path, int type )
{
   QFile file( path );

      if ( file.open( IO_ReadOnly ) )
      {
        QTextStream stream( &file );

        switch( type )
        {
        case 1:
        {
           KURL::List urls = loadM3u( stream, path.left( path.findRev( '/' ) + 1 ) ); //TODO verify that relative playlists work!!
           QApplication::postEvent( m_listView, new PlaylistFoundEvent( path, urls ) );
           break;
        }
        case 2:
           loadPls( stream );
           break;
        default:
           break;
        }
      }

   file.close();
}


bool PlaylistLoader::isValidMedia( const KURL &url, mode_t mode, mode_t permissions ) //static
{
   //FIXME determine if the thing at the end of this is a stream! Can arts do this?
   //      currently we always return true as we can't check
   //FIXME I don't actually understand what checks can be done, etc.
   if( url.protocol() == "http" ) return true;

   //FIXME KMimetype doesn't seem to like http files, so here we are assuming if
   //      it's extension is not common, it can't be read. Not perfect
   //      listed in order of liklihood of encounter to avoid logic checks
   QString ext = url.path().right( 4 ).lower();
   bool b = ( ext == ".mp3" || ext == ".ogg" || ext == ".m3u" || ext == ".pls" || ext == ".mod" ||  ext == ".wav" );

   if( !b && !(b = pApp->m_pEngine->canDecode( url, mode, permissions )) )
       kdDebug() << "Rejected URL: " << url.prettyURL() << endl;

    return b;
}


#ifdef FAST_TRANSLATE
void PlaylistLoader::translate( QString &path )
#else
void PlaylistLoader::translate( QString &path, KFileItemList &list )
#endif
{
   #ifdef FAST_TRANSLATE
   QStringList directories;
   QStringList files;
   #endif

   DIR *d = opendir( path.local8Bit() );
   if( !path.endsWith( "/" ) ) path += '/';

   if( d )
   {
      DIRENT *ent;
      struct STATSTRUCT statbuf;

      while( ( ent = READDIR( d ) ) )
      {
         QString file( ent->d_name );

         if( file == "." || file == ".." ) continue;

         QString newPath = path + file;

         //get file information
         if( LSTAT( newPath.local8Bit(), &statbuf ) == 0 )
         {
            //check for these first as they are not mutually exclusive WRT dir/files
            if( S_ISCHR(  statbuf.st_mode ) ||
                S_ISBLK(  statbuf.st_mode ) ||
                S_ISFIFO( statbuf.st_mode ) ||
                S_ISSOCK( statbuf.st_mode ) ); //then do nothing

            else if( S_ISDIR( statbuf.st_mode ) && options.recurse )  //directory
            {
               if( !options.symlink && S_ISLNK( statbuf.st_mode ) ) continue;
            #ifdef FAST_TRANSLATE
               directories += newPath;
            #else
               translate( newPath, list );
            #endif
            }

            else if( S_ISREG( statbuf.st_mode ) )  //file
            {
               KURL url; url.setPath( newPath );     //safe way to do it for unix paths

               if( isPlaylist( newPath ) )
                  //QApplication::postEvent( m_listView, new PlaylistFoundEvent( url ) );
                  ;
               else
               {

                  //we save some time and pass the stat'd information
                  if( isValidMedia( url, statbuf.st_mode & S_IFMT, statbuf.st_mode & 07777 ) )
                  {
                  #ifdef FAST_TRANSLATE
                      files += file;
                  #else
                      //true means don't determine mimetype (waste of cycles for sure!)
                      list.append( new KFileItem( statbuf.st_mode & S_IFMT, statbuf.st_mode & 07777, url, true ) );
                  #endif
                  }
               }
            }
         } //if( LSTAT )
      } //while

      closedir( d );

      #ifdef FAST_TRANSLATE
      //alpha-sort the files we found, and then post them to the playlist
      files.sort();
      for( QStringList::Iterator it = files.begin(); it != files.end(); ++it )
      {
         (*it).prepend( path );
         KURL url;
         url.setPath( *it );
         postBundle( url );
      }

      //translate all sub-directories
      for( QStringList::Iterator it = directories.begin(); it != directories.end(); ++it ) translate( *it );
      #endif

   } //if( d )
}


KURL::List PlaylistLoader::loadM3u( QTextStream &stream, const QString &dir )
{
    KURL::List urls;

    QString str, title;
    int length = MetaBundle::Undetermined; // = -2

    while( !(str = stream.readLine()).isNull() )
    {
        if( str.startsWith( "#EXTINF" ) )
        {
            QString extinf = str.section( ':', 1, 1 );
            length = extinf.section( ',', 0, 0 ).toInt();
            title  = extinf.section( ',', 1, 1 );

            if( length == 0 ) length = MetaBundle::Undetermined;
        }

        else if( !str.startsWith( "#" ) && !str.isEmpty() )
        {
            if( !( str[0] == '/' || str.startsWith( "http://" ) ) )
                str.prepend( dir );

            KURL url = KURL::fromPathOrURL( str );
            urls.append( url );

            postBundle( url, title, length );

            length = MetaBundle::Undetermined;
            title = QString();
        }
    }

    return urls;
}


void PlaylistLoader::loadPls( QTextStream &stream )
{
    //FIXME algorithm works, but is rather pants!

    for( QString line = stream.readLine(); !line.isNull(); line = stream.readLine() )
    {
        if( line.startsWith( "File" ) )
        {
            KURL url = KURL::fromPathOrURL( line.section( "=", -1 ) );
            QString title;
            int length = 0;

            line = stream.readLine();

            if( line.startsWith( "Title" ) )
            {
                title = line.section( "=", -1 );
                line = stream.readLine();
            }

            if( line.startsWith( "Length" ) )
            {
                length = line.section( "=", -1 ).toInt();
            }

            postBundle( url, title, length );
        }
    }
}



PlaylistItem*
PlaylistLoader::MakeItemEvent::makePlaylistItem( PlaylistWidget *pw )
{
    PlaylistItem *item = new PlaylistItem( pw, m_thread->m_after, m_url, m_title, m_length );
    m_thread->m_after = item;
    return item;
}

PlaylistItem*
PlaylistLoader::DownloadPlaylistEvent::makePlaylistItem( PlaylistWidget *lv )
{
    PlaylistItem *newItem = MakeItemEvent::makePlaylistItem( lv );

    //it is safe to dereference m_thread currently as LoaderThreads are deleted in the main Event Loop
    //and we are blocking the event loop right now!
    //however KIO::NetAccess processes the event loop, so do any m_thread dereferencing before the KIO call

    bool playFirstItem = m_thread->options.playFirstItem;
    m_thread->options.playFirstItem = false; //FIXME what if KIO fails?

    //KIO::NetAccess can make it's own tempfile
    //but we need to add .pls/.m3u extension or the Loader will fail
    QString path = m_url.filename();
    //FIXME KTempfile doesn't allow no prefix AND requires you to add the '.'! whattapileofcrap!
    KTempFile tmpfile( QString::null, path.right( path.findRev( '.' ) ) ); //use default suffix
    path = tmpfile.name();

    kdDebug() << "[PLSloader] Trying to download: " << m_url.prettyURL() << endl;

    //FIXME this will block user input to the interface AND process the event queue
    QApplication::setOverrideCursor( KCursor::waitCursor() );
        bool succeeded = KIO::NetAccess::download( m_url, path, pApp->m_pBrowserWin );
    QApplication::restoreOverrideCursor();

    if( succeeded )
    {
        //the playlist was successfully downloaded
        //KIO::NetAccess created a tempfile, it will be deleted in the new thread's dtor
        KURL url; url.setPath( path ); //required way to set unix paths
        const KURL::List list( url );

        //the item is treated as a placeholder with this ctor
        PlaylistLoader *loader = new PlaylistLoader( list, newItem );
        //pass on the playFirstItem flag to the new loader
        loader->options.playFirstItem = playFirstItem;

        loader->start();

        //FIXME may dereference what has already been deleted!!!! (NOT SAFE!)
        //TODO hide it instead of deleting it and set m_after before hand
        //m_thread->m_after = newItem;
    }
    else
    {
        KMessageBox::sorry( pApp->m_pBrowserWin, i18n( "The playlist, '%1', could not be downloaded." ).arg( m_url.prettyURL() ) );
        newItem->setVisible( false ); //FIXME you set m_thread->m_after to this so we can't delete it!
        tmpfile.unlink();
    }

    //we return 0 because we don't want the Playlist to register this item or try to read its tags
    return 0;
}
