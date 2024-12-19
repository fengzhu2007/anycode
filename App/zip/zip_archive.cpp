#include "zip_archive.h"
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include "minizip/zip.h"
#ifdef Q_OS_LINUX
#include <sys/stat.h>
#endif

#define MAXFILENAME 255

#ifdef _WIN32
        #define USEWIN32IOAPI
        #include "minizip/iowin32.h"
#endif

#ifdef _WIN32
uLong filetime(const char *f,              /* name of file to get info on */
                tm_zip *tmzip,             /* return value: access, modific. and creation times */
                uLong *dt)            /* dostime */
{
  int ret = 0;
  {
      FILETIME ftLocal;
      HANDLE hFind;
      WIN32_FIND_DATAA ff32;

      hFind = FindFirstFileA(f,&ff32);
      if (hFind != INVALID_HANDLE_VALUE)
      {
        FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
        FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        FindClose(hFind);
        ret = 1;
      }
  }
  return ret;
}
#else
#ifdef unix || __APPLE__
uLong filetime(const char *f,              /* name of file to get info on */
                tm_zip *tmzip,             /* return value: access, modific. and creation times */
                uLong *dt)            /* dostime */
{
  int ret=0;
  struct stat s;        /* results of stat() */
  struct tm* filedate;
  time_t tm_t=0;

  if (strcmp(f,"-")!=0)
  {
    char name[MAXFILENAME+1];
    int len = strlen(f);
    if (len > MAXFILENAME)
      len = MAXFILENAME;

    strncpy(name, f,MAXFILENAME-1);
    /* strncpy doesnt append the trailing NULL, of the string is too long. */
    name[ MAXFILENAME ] = '\0';

    if (name[len - 1] == '/')
      name[len - 1] = '\0';
    /* not all systems allow stat'ing a file with / appended */
    if (stat(name,&s)==0)
    {
      tm_t = s.st_mtime;
      ret = 1;
    }
  }
  filedate = localtime(&tm_t);

  tmzip->tm_sec  = filedate->tm_sec;
  tmzip->tm_min  = filedate->tm_min;
  tmzip->tm_hour = filedate->tm_hour;
  tmzip->tm_mday = filedate->tm_mday;
  tmzip->tm_mon  = filedate->tm_mon ;
  tmzip->tm_year = filedate->tm_year;

  return ret;
}
#else
uLong filetime(const char *f,              /* name of file to get info on */
                tm_zip *tmzip,             /* return value: access, modific. and creation times */
                uLong *dt)             /* dostime */
{
    return 0;
}
#endif
#endif


#ifdef __APPLE__
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#else
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif

#define WRITEBUFFERSIZE (16384)

namespace ady
{
    ZipArchive::ZipArchive(QObject* parent)
        :QObject(parent)
    {
        m_zf = nullptr;
        m_errorCode = 0;
    }

    bool ZipArchive::open(QString pathname,Mode mode)
    {
        if(mode!=Normal){
            QFileInfo fi(pathname);
            if(!fi.exists()){
                mode = Normal;
            }
        }

#       ifdef USEWIN32IOAPI
            zlib_filefunc64_def ffunc;
            fill_win32_filefunc64A(&ffunc);
            //(opt_overwrite==2) ? 2 : 0
            //int mode = 0;
            qDebug()<<"path:"<<pathname;
            m_zf = zipOpen2_64(pathname.toLocal8Bit(),(int)mode,NULL,&ffunc);
#       else
            m_zf = zipOpen64(pathname.toLocal8Bit(),mode);
#       endif
        if(m_zf==nullptr){
            m_errorCode = ZIP_ERRNO;
            return false;
        }else{
            return true;
        }
    }

    bool ZipArchive::open(QString pathname,QString password,Mode mode)
    {
        m_password = password;
        return this->open(pathname,mode);
    }

    bool ZipArchive::addFile(QString pathname,QString filename)
    {
        /*FILE * fin;
        int size_read;
        int size_buf=0;
        void* buf=nullptr;*/
        int opt_compress_level=Z_DEFAULT_COMPRESSION;
        zip_fileinfo zi;
        zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
        zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
        zi.dosDate = 0;
        zi.internal_fa = 0;
        zi.external_fa = 0;
        unsigned long crcFile=0;
        int zip64 = 0;
        filetime(pathname.toLocal8Bit(),&zi.tmz_date,&zi.dosDate);
        if(m_password.isNull() || m_password.isEmpty()){
            m_errorCode = zipOpenNewFileInZip3_64(m_zf,filename.toUtf8(),&zi,
                                             NULL,0,NULL,0,NULL /* comment*/,
                                             (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                             opt_compress_level,0,
                                             /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
                                             -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                             NULL,crcFile, zip64);
        }else{
            m_errorCode = zipOpenNewFileInZip3_64(m_zf,filename.toUtf8(),&zi,
                                             NULL,0,NULL,0,NULL /* comment*/,
                                             (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                             opt_compress_level,0,
                                             /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
                                             -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                             m_password.toUtf8(),crcFile, zip64);
        }


        if(m_errorCode==ZIP_OK){
            QFile file(pathname);
            if(file.open(QIODevice::ReadOnly)){
                //QByteArray ba;
                int read_size = 0;
                do{
                    //ba = file.read(WRITEBUFFERSIZE);
                    char* buf = new char[WRITEBUFFERSIZE];
                    read_size = file.read(buf,WRITEBUFFERSIZE);
                    m_errorCode = zipWriteInFileInZip (m_zf,buf,read_size);

                }while(read_size>0 &&m_errorCode == ZIP_OK);
                //QByteArray ba = file.readAll();
                file.close();
                if (m_errorCode<0){
                    m_errorCode=ZIP_ERRNO;
                }else{
                    m_errorCode = zipCloseFileInZip(m_zf);
                }
                return m_errorCode == ZIP_OK;
            }else{
                m_errorCode=ZIP_ERRNO;
                return false;
            }
        }else{
            return false;
        }

        //todo:write replace qt style
        /*if(m_errorCode==ZIP_OK){
            fin = FOPEN_FUNC(pathname.toLocal8Bit(),"rb");
            if(fin==nullptr){
                m_errorCode = ZIP_ERRNO;
                return false;
            }
            bool result = true;
            buf = (void*)malloc(size_buf);
            do{
                m_errorCode = ZIP_OK;
                size_read = (int)fread(buf,1,size_buf,fin);
                if (size_read < size_buf)
                if (feof(fin)==0)
                {
                    m_errorCode = ZIP_ERRNO;
                    result = false;
                    break;
                }
                if (size_read>0)
                {
                    m_errorCode = zipWriteInFileInZip (m_zf,buf,size_read);
                    if (m_errorCode<0)
                    {
                        result = false;
                        break;
                    }

                }
            }while((m_errorCode == ZIP_OK) && (size_read>0));

            if (fin!=nullptr)
                fclose(fin);
            if (m_errorCode<0){
                m_errorCode=ZIP_ERRNO;
            }else{
                m_errorCode = zipCloseFileInZip(m_zf);
            }
            return result;
        }else{
            return false;
        }*/


    }

    bool ZipArchive::addContent(QByteArray ba,QString filename)
    {
        return false;
    }

    bool ZipArchive::addFolder(QString dirname)
    {
        return false;
    }

    bool ZipArchive::close()
    {
        m_errorCode = zipClose(m_zf,NULL);
        return m_errorCode == ZIP_OK;
    }


}
