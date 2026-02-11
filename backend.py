from flask import Flask, request, Response
import requests
from urllib.parse import urlparse, urljoin
import logging

app = Flask(__name__)

# Настройка логирования
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Заголовки, которые нужно пробросить
FORWARDED_HEADERS = [
    'User-Agent', 'Accept', 'Accept-Encoding', 'Accept-Language',
    'Cache-Control', 'Connection', 'Cookie', 'Host', 'Referer',
    'Content-Type', 'Content-Length', 'Authorization'
]

def get_request_headers():
    """Получить заголовки из входящего запроса"""
    headers = {}
    for header in FORWARDED_HEADERS:
        value = request.headers.get(header)
        if value:
            headers[header] = value
    return headers

@app.route('/', defaults={'path': ''}, methods=['GET', 'POST', 'PUT', 'DELETE', 'PATCH', 'OPTIONS', 'HEAD'])
@app.route('/<path:path>', methods=['GET', 'POST', 'PUT', 'DELETE', 'PATCH', 'OPTIONS', 'HEAD'])
def proxy(path):
    """Прокси для всех запросов"""
    
    # Получаем целевой URL из заголовков или параметров
    target_url = request.url.replace(request.host_url, '')
    
    # Если это не полный URL, пытаемся его построить
    if not target_url.startswith('http'):
        # Проверяем, есть ли хост в заголовках
        host = request.headers.get('Host')
        if host and host != request.host:
            scheme = 'https' if request.is_secure else 'http'
            target_url = f"{scheme}://{target_url}"
        else:
            logger.warning(f"Невалидный URL: {target_url}")
            return Response("Invalid URL", status=400)
    
    logger.info(f"Проксируем запрос: {request.method} {target_url}")
    
    try:
        # Получаем заголовки
        headers = get_request_headers()
        
        # Получаем данные тела запроса, если есть
        data = None
        if request.method in ['POST', 'PUT', 'PATCH']:
            data = request.get_data()
        
        # Делаем запрос к целевому серверу
        response = requests.request(
            method=request.method,
            url=target_url,
            headers=headers,
            data=data,
            cookies=request.cookies,
            allow_redirects=False,
            stream=True,
            timeout=30
        )
        
        # Создаем ответ
        excluded_headers = ['content-encoding', 'content-length', 'transfer-encoding', 'connection']
        response_headers = [
            (name, value) for (name, value) in response.raw.headers.items()
            if name.lower() not in excluded_headers
        ]
        
        # Добавляем CORS заголовки
        response_headers.append(('Access-Control-Allow-Origin', '*'))
        response_headers.append(('Access-Control-Allow-Methods', 'GET, POST, PUT, DELETE, OPTIONS'))
        response_headers.append(('Access-Control-Allow-Headers', 'Content-Type, Authorization'))
        
        flask_response = Response(
            response.content,
            status=response.status_code,
            headers=response_headers
        )
        
        logger.info(f"Ответ: {response.status_code} для {target_url}")
        return flask_response
        
    except requests.exceptions.RequestException as e:
        logger.error(f"Ошибка при запросе к {target_url}: {str(e)}")
        return Response(f"Proxy Error: {str(e)}", status=502)
    except Exception as e:
        logger.error(f"Неожиданная ошибка: {str(e)}")
        return Response(f"Internal Server Error: {str(e)}", status=500)

@app.route('/health')
def health():
    """Проверка работоспособности сервера"""
    return {"status": "ok", "message": "Proxy server is running"}

if __name__ == '__main__':
    print("=" * 50)
    print("🚀 Прокси-сервер запущен на http://localhost:5000")
    print("=" * 50)
    print("Настройте браузер на использование прокси:")
    print("  - HTTP Proxy: localhost")
    print("  - Порт: 5000")
    print("=" * 50)
    
    app.run(
        host='0.0.0.0',
        port=5000,
        debug=True,
        threaded=True
    )
