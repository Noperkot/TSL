# TSL 1.7.1  
### Лаунчер для [TorrServer](https://github.com/YouROK/TorrServer) под Windows  
Позволяет сворачивать консольное окно торрсервера в трей. Есть автозапуск при входе в систему. Работает с любой версией сервера.  
#### Использование:  
[**tsl.exe**](../../releases/latest/download/tsl.exe) положить в папку с исполняемым файлом торрсервера _[(TorrServer-windows-\*\*\*.exe)](https://github.com/YouROK/TorrServer/releases)_ и запустить. Торрсервер стартует свернутым в трей (иконка рядом с часами в правом нижнем углу экрана). При желании можно включить автозапуск.   

<details>
<summary>Параметры запуска tsl.exe (для 99.99% пользователей это НЕ НУЖНО)</summary>  
  
#### Параметры командной строки:  
  
  Команда | Если tsl уже запущен | В противном случае
------------ | ------------- | -------------
--close | Закрыть запущенный экземпляр tsl | ничего не делает
--stop | Остановить сервер в запущенном экземпляре tsl | tsl стартует с остановленным сервером
--start | Запуск сервера в запущенном экземпляре tsl | tsl запускается свернутым в трей
--restart | Рестарт сервера в запущенном экземпляре tsl | tsl запускается свернутым в трей
--show | Развернуть окно запущенного экземпляра tsl | tsl запускается с открытым окном
--hide | Свернуть окно запущенного экземпляра tsl в трей | tsl запускается свернутым в трей
--reset | Закрыть запущенный экземпляр tsl и сбросить параметры реестра | Сброс параметров реестра
--web | Открыть в браузере веб-интерфейс TS | tsl запускается свернутым в трей и открывается веб-интерфейс TS
  
#### Параметры реестра (HKEY_CURRENT_USER\Software\TorrServer):  
  
 Параметр | Тип | Дефолтное значение | Описание
------------ | ------------- | ------------- | -------------
WindowX | REG_DWORD | автоцентрирование | X координата окна (если создать этот параметр, он будет запоминаться при выходе)
WindowY | REG_DWORD | автоцентрирование | Y координата окна (если создать этот параметр, он будет запоминаться при выходе)
WindowW | REG_DWORD | 2/3 экрана | Ширина окна (запоминается при выходе)
WindowH | REG_DWORD | 2/3 экрана | Высота окна (запоминается при выходе)
WindowMinW | REG_DWORD | 320 | Минимальная ширина окна
WindowMinH | REG_DWORD | 240 | Минимальная высота окна
WindowMax | REG_DWORD | 0 | Развернуть на весь экран. 0-нет, 1-да (запоминается при выходе)
TextWrapping | REG_DWORD | 0 | Переносить строки в консоли. 0-нет, 1-да
MaxLines | REG_DWORD | 1000 | Строк в кольцевом буфере консоли
ExitWhenClose | REG_DWORD | 0 | Действие при закрытии окна. 0 - свернуть в трей, 1 - выход
OnTSdead | REG_DWORD | 0 | Действие при падении TS. 0 - ничего не делать, 1 - закрыть программу, 2 - развернуть окно программы, 3 - перезапустить TS
OnIconClick | REG_DWORD | 0 | Действие при клике по иконке в трее. 0 - показать/скрыть окно, 1 - открыть веб-интерфейс TS, 2 - рестарт TS
DblIconClick | REG_DWORD | 0 | Какой обрабатывать клик по иконке в трее. 0 - одинарный, 1 - двойной
ConsoleBkColor | REG_DWORD | 0x000000 | hex цвет фона 0xRRGGBB
ConsoleFontColor | REG_DWORD | 0xBBBBBB | hex цвет шрифта 0xRRGGBB
ConsoleFontSize | REG_DWORD | 9 | Размер шрифта
ConsoleFontName | REG_SZ | Lucida Console | Название шрифта
args | REG_SZ |  | Аргументы командной строки TS

</details>

> Для ленивых - [**инсталлятор**](../../releases/latest/download/TorrServer_MatriX.116_setup.exe) (TorrServer MatriX.116 + лаунчер).  
> Для работы с сервером можно использовать любой MatriX-совместимый TorrServe клиент, в том числе расширение для браузеров **TorrServer Adder** ( [Chrome](https://chrome.google.com/webstore/detail/torrserver-adder/ihphookhabmjbgccflngglmidjloeefg?hl=ru), [Firefox](https://addons.mozilla.org/ru/firefox/addon/torrserver-adder/) ).  

Обсуждение TorrServer'а и всего, что с ним связано:
- [на 4PDA](https://4pda.to/forum/index.php?showtopic=889960)
- [в Telegram](https://t.me/TorrServe)
***
<details>
<summary>Скриншоты</summary>  

![](https://raw.githubusercontent.com/Noperkot/TSL/master/img/screen1.png)  

![](https://raw.githubusercontent.com/Noperkot/TSL/master/img/screen2.png)  
</details>