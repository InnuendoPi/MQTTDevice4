String unsupportedFiles = String();

// Utils to return HTTP codes, and determine content-type

void replyOK()
{
  server.send(200, FPSTR("text/plain"), "");
}

void replyOKWithMsg(String msg)
{
  server.send(200, FPSTR("text/plain"), msg);
}

void replyNotFound(String msg)
{
  server.send(404, FPSTR("text/plain"), msg);
}

void replyBadRequest(String msg)
{
  server.send(400, FPSTR("text/plain"), msg + "\r\n");
}

void replyServerError(String msg)
{
  server.send(500, FPSTR("text/plain"), msg + "\r\n");
}

String getContentType(const String &filename)
{
  if (server.hasArg("download"))
  {
    return "application/octet-stream";
  }
  else if (filename.endsWith(".htm"))
  {
    return "text/html";
  }
  else if (filename.endsWith(".html"))
  {
    return "text/html";
  }
  else if (filename.endsWith(".css"))
  {
    return "text/css";
  }
  else if (filename.endsWith(".js"))
  {
    return "application/javascript";
  }
  else if (filename.endsWith(".png"))
  {
    return "image/png";
  }
  else if (filename.endsWith(".gif"))
  {
    return "image/gif";
  }
  else if (filename.endsWith(".jpg"))
  {
    return "image/jpeg";
  }
  else if (filename.endsWith(".ico"))
  {
    return "image/x-icon";
  }
  else if (filename.endsWith(".xml"))
  {
    return "text/xml";
  }
  else if (filename.endsWith(".pdf"))
  {
    return "application/x-pdf";
  }
  else if (filename.endsWith(".zip"))
  {
    return "application/x-zip";
  }
  else if (filename.endsWith(".gz"))
  {
    return "application/x-gzip";
  }
  else if (filename.endsWith(".json"))
  {
    return "text/plain";
  }
  else if (filename.endsWith(".mp3"))
  {
    return "audio/mpeg3";
  }

  return "text/plain";
}

void handleStatus()
{
  String json;
  json.reserve(128);
  json = "{\"type\":\"Filesystem\", \"isOk\":";
  json += PSTR("\"true\", \"totalBytes\":\"");
#ifdef ESP32
  json += LittleFS.totalBytes();
#elif ESP8266
  FSInfo fs_info;
  LittleFS.info(fs_info);
  json += fs_info.totalBytes;
#endif
  json += PSTR("\", \"usedBytes\":\"");
#ifdef ESP32
  json += LittleFS.usedBytes();
#elif ESP8266
  json += fs_info.usedBytes;
#endif
  json += "\"";
  json += PSTR(",\"unsupportedFiles\":\"\"}");
  server.send(200, "application/json", json);
}

bool exists(String path)
{
  File file = LittleFS.open(path, "r");
  if (!file.isDirectory())
    return true;

  file.close();
  return false;
}

#ifdef ESP32
void handleFileList()
{
  if (!server.hasArg("dir"))
  {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }

  String path = server.arg("dir");
  File root = LittleFS.open(path);
  path.clear();
  String output = "[";
  if (root.isDirectory())
  {
    File file = root.openNextFile();
    while (file)
    {
      if (output != "[")
      {
        output += ',';
      }
      output += "{\"type\":\"";
      if (file.isDirectory())
        output += "dir";
      else
      {
        output += F("file\",\"size\":\"");
        output += file.size();
      }
      output += "\",\"name\":\"";
      if (file.name()[0] == '/')
        output += &(file.name()[1]);
      else
        output += file.name();

      output += "\"}";
      file = root.openNextFile();
    }
  }
  output += "]";
  server.send(200, "text/json", output);
}
#elif ESP8266
void handleFileList()
{
  if (!server.hasArg("dir"))
  {
    return replyBadRequest(F("DIR ARG MISSING"));
  }

  String path = server.arg("dir");
  if (path != "/" && LittleFS.mkdir(path)) // mkdir workarround für exists on directory
  {
    return replyBadRequest("BAD PATH");
  }

  Dir dir = LittleFS.openDir(path);
  path.clear();
  if (!server.chunkedResponseModeStart(200, "text/json"))
  {
    server.send(505, F("text/html"), F("HTTP1.1 required"));
    return;
  }

  String output;
  output.reserve(64);
  while (dir.next())
  {
    if (output.length())
    {
      server.sendContent(output);
      output = ',';
    }
    else
    {
      output = '[';
    }

    output += "{\"type\":\"";
    if (dir.isDirectory())
    {
      output += "dir";
    }
    else
    {
      output += F("file\",\"size\":\"");
      output += dir.fileSize();
    }

    output += F("\",\"name\":\"");
    if (dir.fileName()[0] == '/')
    {
      output += &(dir.fileName()[1]);
    }
    else
    {
      output += dir.fileName();
    }

    output += "\"}";
  }
  output += "]";
  server.sendContent(output);
  server.chunkedResponseFinalize();
}
#endif

//   Read the given file from the filesystem and stream it back to the client

bool handleFileRead(String path)
{
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (exists(pathWithGz) || exists(path))
  {
    if (exists(pathWithGz))
    {
      path += ".gz";
    }
    File file = LittleFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

// As some FS (e.g. LittleFS) delete the parent folder when the last child has been removed, return the path of the closest parent still existing

String lastExistingParent(String path)
{
  while (!path.isEmpty() && !LittleFS.exists(path))
  {
    if (path.lastIndexOf('/') > 0)
      path = path.substring(0, path.lastIndexOf('/'));
    else
      path = String(); // No slash => the top folder does not exist
  }
  return path;
}

/*
   Handle the creation/rename of a new file
   Operation      | req.responseText
   ---------------+--------------------------------------------------------------
   Create file    | parent of created file
   Create folder  | parent of created folder
   Rename file    | parent of source file
   Move file      | parent of source file, or remaining ancestor
   Rename folder  | parent of source folder
   Move folder    | parent of source folder, or remaining ancestor
*/

void handleFileCreate()
{
  String path = server.arg("path");
  if (path.isEmpty())
    return replyBadRequest(F("PATH ARG MISSING"));
  if (path == "/")
    return replyBadRequest("BAD PATH");
  if (LittleFS.exists(path))
    return replyBadRequest(F("PATH FILE EXISTS"));

  String src = server.arg("src");
  if (src.isEmpty())
  {
    // No source specified: creation
    if (path.endsWith("/"))
    {
      // Create a folder
      path.remove(path.length() - 1);
      if (!LittleFS.mkdir(path))
        return replyServerError(F("MKDIR FAILED"));
    }
    else
    {
      // Create a file
      File file = LittleFS.open(path, "w");
      if (file)
        file.close();
      else
        return replyServerError(F("CREATE FAILED"));
    }
    if (path.lastIndexOf('/') > -1)
      path = path.substring(0, path.lastIndexOf('/'));

    replyOKWithMsg(path);
  }
  else
  {
    // Source specified: rename
    if (src == "/")
      return replyBadRequest("BAD SRC");
    if (!LittleFS.exists(src))
      return replyBadRequest(F("SRC FILE NOT FOUND"));

    if (path.endsWith("/"))
      path.remove(path.length() - 1);
    if (src.endsWith("/"))
      src.remove(src.length() - 1);
    if (!LittleFS.rename(src, path))
      return replyServerError(F("RENAME FAILED"));
    replyOKWithMsg(lastExistingParent(src));
  }
}

void deleteRecursive(String path)
{
  File file = LittleFS.open(path, "r");
  bool isDir = file.isDirectory();
  file.close();
  if (!isDir)
  {
    LittleFS.remove(path);
    return;
  }
  LittleFS.rmdir(path);
}

/*
   Handle a file deletion request
   Operation      | req.responseText
   ---------------+--------------------------------------------------------------
   Delete file    | parent of deleted file, or remaining ancestor
   Delete folder  | parent of deleted folder, or remaining ancestor
*/
void handleFileDelete()
{
  String path = server.arg(0);
  if (path.isEmpty() || path == "/")
    return replyBadRequest("BAD PATH");

  if (!LittleFS.exists(path))
    return replyNotFound(FPSTR("FileNotFound"));
  deleteRecursive(path);
  replyOKWithMsg(lastExistingParent(path));
}

// Handle a file upload request

void handleFileUpload()
{
  if (server.uri() != "/edit")
    return;
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START)
  {
    String filename = upload.filename;
    if (!filename.startsWith("/"))
      filename = "/" + filename;
    fsUploadFile = LittleFS.open(filename, "w");
    if (!fsUploadFile)
      return replyServerError(F("CREATE FAILED"));
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    if (fsUploadFile)
    {
      size_t bytesWritten = fsUploadFile.write(upload.buf, upload.currentSize);
      if (bytesWritten != upload.currentSize)
        return replyServerError(F("WRITE FAILED"));
    }
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (fsUploadFile)
      fsUploadFile.close();
  }
}

void handleGetEdit()
{
  server.sendHeader(PSTR("Content-Encoding"), "gzip");
  server.send_P(200, "text/html", edit_htm_gz, edit_htm_gz_len);
}
