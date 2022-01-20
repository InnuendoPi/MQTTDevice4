void replyToCLient(int msg_type = 0, const char *msg = "")
{
  server.sendHeader("Access-Control-Allow-Origin", "*");
  switch (msg_type)
  {
  case OK:
    server.send(200, FPSTR(TEXT_PLAIN), "");
    break;
  case CUSTOM:
    server.send(200, FPSTR(TEXT_PLAIN), msg);
    break;
  case NOT_FOUND:
    server.send(404, FPSTR(TEXT_PLAIN), msg);
    break;
  case BAD_REQUEST:
    server.send(400, FPSTR(TEXT_PLAIN), msg);
    break;
  case ERROR:
    server.send(500, FPSTR(TEXT_PLAIN), msg);
    break;
  }
}

void replyOK()
{
  replyToCLient(OK, "");
}

void handleGetEdit()
{
  server.sendHeader(PSTR("Content-Encoding"), "gzip");
  server.send_P(200, "text/html", edit_htm_gz, sizeof(edit_htm_gz));
}

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

void handleFileList()
{
  if (!server.hasArg("dir"))
  {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }
  String path = server.arg("dir");
  Dir dir = LittleFS.openDir(path);
  path = String();

  String output = "[";
  while (dir.next())
  {
    File entry = dir.openFile("r");
    if (output != "[")
    {
      output += ',';
    }
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir) ? "dir" : "file";
    output += "\",\"size\":\"";
    output += entry.size();
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(0);
    output += "\"}";
    entry.close();
  }
  output += "]";
  server.send(200, "text/json", output);
}

void checkForUnsupportedPath(String &filename, String &error)
{
  if (!filename.startsWith("/"))
  {
    error += PSTR(" !! NO_LEADING_SLASH !! ");
  }
  if (filename.indexOf("//") != -1)
  {
    error += PSTR(" !! DOUBLE_SLASH !! ");
  }
  if (filename.endsWith("/"))
  {
    error += PSTR(" ! TRAILING_SLASH ! ");
  }
}

// format bytes
String formatBytes(size_t bytes)
{
  if (bytes < 1024)
  {
    return String(bytes) + "B";
  }
  else if (bytes < (1024 * 1024))
  {
    return String(bytes / 1024.0) + "KB";
  }
  else if (bytes < (1024 * 1024 * 1024))
  {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
  else
  {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}

String getContentType(String filename)
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
  else if (filename.endsWith(".sass"))
  {
    return "text/css";
  }
  else if (filename.endsWith(".js"))
  {
    return "application/javascript";
  }
  else if (filename.endsWith(".png"))
  {
    return "image/svg+xml";
  }
  else if (filename.endsWith(".svg"))
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
  return "text/plain";
}

// Datei editieren -> speichern CTRL+S
bool handleFileRead(String path)
{
  if (path.endsWith("/"))
  {
    path += "index.html";
  }
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (LittleFS.exists(pathWithGz) || LittleFS.exists(path))
  {
    if (LittleFS.exists(pathWithGz))
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

void handleFileUpload()
{
  if (server.uri() != "/edit")
  {
    return;
  }
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START)
  {
    String filename = upload.filename;
    String result;
    // Make sure paths always start with "/"
    if (!filename.startsWith("/"))
    {
      filename = "/" + filename;
    }
    checkForUnsupportedPath(filename, result);
    if (result.length() > 0)
    {
      replyToCLient(ERROR, PSTR("INVALID FILENAME"));
      return;
    }
    DEBUG_MSG("FS: file name upload: %s\n", filename.c_str());
    fsUploadFile = LittleFS.open(filename, "w");
    if (!fsUploadFile)
    {
      replyToCLient(ERROR, PSTR("CREATE FAILED"));
      return;
    }
    filename = String();
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    DEBUG_MSG("FS file size pload: %d\n", upload.currentSize);
    if (fsUploadFile)
    {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (fsUploadFile)
    {
      fsUploadFile.close();
    }
    DEBUG_MSG("FS: upload size: %d\n", upload.totalSize);
    loadConfig();
  }
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
  if (server.args() == 0)
  {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  String path = server.arg(0);
  if (!LittleFS.exists(path))
  {
    replyToCLient(NOT_FOUND, PSTR(FILE_NOT_FOUND));
    return;
  }
  //deleteRecursive(path);
  File root = LittleFS.open(path, "r");
  // If it's a plain file, delete it
  if (!root.isDirectory())
  {
    root.close();
    LittleFS.remove(path);
    replyOK();
  }
  else
  {
    LittleFS.rmdir(path);
    replyOK();
  }
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
  {
    replyToCLient(BAD_REQUEST, PSTR("PATH ARG MISSING"));
    return;
  }
  if (path == "/")
  {
    replyToCLient(BAD_REQUEST, PSTR("BAD PATH"));
    return;
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
        replyToCLient(ERROR, PSTR("MKDIR FAILED"));
        return;
      }
    }
    else
    {
      // Create a file
      File file = LittleFS.open(path, "w");
      if (file)
      {
        file.write(0);
        file.close();
      }
      else
      {
        replyToCLient(ERROR, PSTR("CREATE FAILED"));
        return;
      }
    }
    replyToCLient(CUSTOM, path.c_str());
  }
  else
  {
    // Source specified: rename
    if (src == "/")
    {
      replyToCLient(BAD_REQUEST, PSTR("BAD SRC"));
      return;
    }

    if (!LittleFS.exists(src))
    {
      replyToCLient(BAD_REQUEST, PSTR("BSRC FILE NOT FOUND"));
      return;
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
      replyToCLient(ERROR, PSTR("RENAME FAILED"));
      return;
    }
    replyOK();
  }
}
