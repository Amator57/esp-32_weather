server.on("/restart", HTTP_POST, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", R"rawliteral(
      <html>
      <head>
        <meta charset="UTF-8">
        <title>Перезавантаження</title>
        <script>
          setTimeout(function() {
            window.location.href = "/";
          }, 8000); // почекай 8 секунд і перенаправ
        </script>
      </head>
      <body style="font-family: sans-serif; text-align: center; padding-top: 50px;">
        <h2>🔁 Пристрій перезавантажується...</h2>
        <p>Будь ласка, зачекайте кілька секунд.</p>
      </body>
      </html>
    )rawliteral");
    delay(500); // дати час відповіді HTTP
    ESP.restart();
  });