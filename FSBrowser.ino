String unsupportedFiles = String();

////////////////////////////////
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

////////////////////////////////
// Request handlers

/*
   Return the FS type, status and size info
*/
void handleStatus()
{
  FSInfo fs_info;
  LittleFS.info(fs_info);
  String json;
  json.reserve(128);
  json = "{\"type\":\"Filesystem\", \"isOk\":";
  json += PSTR("\"true\", \"totalBytes\":\"");
  json += fs_info.totalBytes;
  json += PSTR("\", \"usedBytes\":\"");
  json += fs_info.usedBytes;
  json += "\"";
  json += PSTR(",\"unsupportedFiles\":\"\"}");
  server.send(200, "application/json", json);
}

/*
   Return the list of files in the directory specified by the "dir" query string parameter.
   Also demonstrates the use of chunked responses.
*/
void handleFileList()
{
  // if (!fsOK) { return replyServerError(FPSTR("FS INIT ERROR")); }

  if (!server.hasArg("dir"))
  {
    return replyBadRequest(F("DIR ARG MISSING"));
  }

  String path = server.arg("dir");
  // if (path != "/" && !fileSystem->exists(path))
  // if (path != "/" && LittleFS.exists(path))
  if (path != "/" && LittleFS.mkdir(path)) // mkdir workarround für exists on directory
  {
    // Serial.printf("FSBrowser path: %s\n", path.c_str());
    return replyBadRequest("BAD PATH");
  }

  Dir dir = LittleFS.openDir(path);
  path.clear();

  // use HTTP/1.1 Chunked response to avoid building a huge temporary string
  if (!server.chunkedResponseModeStart(200, "text/json"))
  {
    server.send(505, F("text/html"), F("HTTP1.1 required"));
    return;
  }

  // use the same string for every line
  String output;
  output.reserve(64);
  while (dir.next())
  {
    if (output.length())
    {
      // send string from previous iteration
      // as an HTTP chunk
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
    // Always return names without leading "/"
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

  // send last string
  output += "]";
  server.sendContent(output);
  server.chunkedResponseFinalize();
}

/*
   Read the given file from the filesystem and stream it back to the client
*/
bool handleFileRead(String path)
{
  // if (!fsOK) {
  //   replyServerError(FPSTR("FS INIT ERROR"));
  //   return true;
  // }

  if (path.endsWith("/"))
  {
    // path += "index.htm";
    path += "index.html";
  }

  String contentType;
  if (server.hasArg("download"))
  {
    contentType = F("application/octet-stream");
  }
  else
  {
    contentType = mime::getContentType(path);
  }
  if (!LittleFS.exists(path))
  {
    // File not found, try gzip version
    path = path + ".gz";
  }
  if (LittleFS.exists(path))
  {
    File file = LittleFS.open(path, "r");
    if (server.streamFile(file, contentType) != file.size()) { Serial.println("Sent less data than expected!"); }
    file.close();
    return true;
  }

  return false;
}

/*
   As some FS (e.g. LittleFS) delete the parent folder when the last child has been removed,
   return the path of the closest parent still existing
*/
String lastExistingParent(String path)
{
  while (!path.isEmpty() && !LittleFS.exists(path))
  {
    if (path.lastIndexOf('/') > 0)
    {
      path = path.substring(0, path.lastIndexOf('/'));
    }
    else
    {
      path = String(); // No slash => the top folder does not exist
    }
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
  // if (!fsOK) { return replyServerError(FPSTR("FS INIT ERROR")); }

  String path = server.arg("path");
  if (path.isEmpty())
  {
    return replyBadRequest(F("PATH ARG MISSING"));
  }

  if (path == "/")
  {
    return replyBadRequest("BAD PATH");
  }
  if (LittleFS.exists(path))
  {
    return replyBadRequest(F("PATH FILE EXISTS"));
  }

  String src = server.arg("src");
  if (src.isEmpty())
  {
    // No source specified: creation
    if (path.endsWith("/"))
    {
      // Create a folder
      path.remove(path.length() - 1);
      if (!LittleFS.mkdir(path))
      {
        return replyServerError(F("MKDIR FAILED"));
      }
    }
    else
    {
      // Create a file
      // File file = fileSystem->open(path, "w");
      File file = LittleFS.open(path, "w");
      if (file)
      {
        file.write((const char *)0);
        file.close();
      }
      else
      {
        return replyServerError(F("CREATE FAILED"));
      }
    }
    if (path.lastIndexOf('/') > -1)
    {
      path = path.substring(0, path.lastIndexOf('/'));
    }
    replyOKWithMsg(path);
  }
  else
  {
    // Source specified: rename
    if (src == "/")
    {
      return replyBadRequest("BAD SRC");
    }
    if (!LittleFS.exists(src))
    {
      return replyBadRequest(F("SRC FILE NOT FOUND"));
    }

    if (path.endsWith("/"))
    {
      path.remove(path.length() - 1);
    }
    if (src.endsWith("/"))
    {
      src.remove(src.length() - 1);
    }
    if (!LittleFS.rename(src, path))
    {
      return replyServerError(F("RENAME FAILED"));
    }
    replyOKWithMsg(lastExistingParent(src));
  }
}

/*
   Delete the file or folder designed by the given path.
   If it's a file, delete it.
   If it's a folder, delete all nested contents first then the folder itself

   IMPORTANT NOTE: using recursion is generally not recommended on embedded devices and can lead to crashes (stack overflow errors).
   This use is just for demonstration purpose, and FSBrowser might crash in case of deeply nested filesystems.
   Please don't do this on a production system.
*/
void deleteRecursive(String path)
{
  File file = LittleFS.open(path, "r");
  bool isDir = file.isDirectory();
  file.close();

  // If it's a plain file, delete it
  if (!isDir)
  {
    LittleFS.remove(path);
    return;
  }

  // Otherwise delete its contents first
  Dir dir = LittleFS.openDir(path);

  while (dir.next())
  {
    deleteRecursive(path + '/' + dir.fileName());
  }

  // Then delete the folder itself
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
  // if (!fsOK) { return replyServerError(FPSTR("FS INIT ERROR")); }

  String path = server.arg(0);
  if (path.isEmpty() || path == "/")
  {
    return replyBadRequest("BAD PATH");
  }

  if (!LittleFS.exists(path))
  {
    return replyNotFound(FPSTR("FileNotFound"));
  }
  deleteRecursive(path);

  replyOKWithMsg(lastExistingParent(path));
}

/*
   Handle a file upload request
*/

void handleFileUpload()
{
  // if (!fsOK) { return replyServerError(FPSTR("FS INIT ERROR")); }
  if (server.uri() != "/edit")
  {
    return;
  }
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START)
  {
    String filename = upload.filename;
    // Make sure paths always start with "/"
    if (!filename.startsWith("/"))
    {
      filename = "/" + filename;
    }
    fsUploadFile = LittleFS.open(filename, "w");
    if (!fsUploadFile)
    {
      return replyServerError(F("CREATE FAILED"));
    }
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    if (fsUploadFile)
    {
      size_t bytesWritten = fsUploadFile.write(upload.buf, upload.currentSize);
      if (bytesWritten != upload.currentSize)
      {
        return replyServerError(F("WRITE FAILED"));
      }
    }
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (fsUploadFile)
    {
      fsUploadFile.close();
    }
  }
}

void handleGetEdit()
{
  //  if (handleFileRead(F("/edit.htm"))) { return; }
  server.sendHeader(PSTR("Content-Encoding"), "gzip");
  server.send_P(200, "text/html", edit_htm_gz, edit_htm_gz_len);
}
