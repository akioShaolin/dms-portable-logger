# Interface web

Cada firmware cria AP local e atende em `http://192.168.4.1/`. A página não usa CDN. Endpoints implementados: `/api/status`, `/api/live`, `/api/logs`, download e CSV em `/api/logs/{name}` e `/csv`, `POST /api/time`, `POST /api/session/rotate`, `DELETE /api/logs/{name}` e `POST /api/logs/erase-all` com corpo `ERASE-ALL`.

Nomes são rejeitados quando contêm `..`, barra invertida ou extensão diferente de `.dmslog`. A senha não aparece nas APIs. Downloads/CSV rodam na tarefa web do Core 0.
