# Mini-PGW

## Описание
Mini-pgw tralalelo tralala

## Сборка и запуск
```bash
build.sh
```

### Запуск сервера
```bash
start
```

### Перезагрузка конфига
```bash
curl http://localhost:8080/reload_config
curl http://localhost:8080/check_subscriber?imsi=123456789123456
curl http://localhost:8080/stop
```

### Запуск клиента
```bash
./pgw_client 001010123456789
```

### Запуск unit-тестов
```bash
ctest
```

### Запуск нагрузочного теста
```bash
./load_test
```
