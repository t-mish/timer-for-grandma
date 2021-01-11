# Таймер с регулируемым временем простоя и временем работы

![228](https://i.imgur.com/PvdL2hR.jpg)

### Предыстория

У моей бабушки на участок вода не поступает через трубопровод. У нее есть скважина (колодец) определенной глубины. Там с течением определенного времени скапливается вода, которую с помощью насоса затем поднимают на поверхность. Ей либо заполняют бочку либо сразу начинают поливать.

Однако воду нельзя постоянно поднимать на поверхность поскольку вода кончается. Нужно ждать ее накопления. Если оставлять насос включенным постоянно, то он будет работать вхолостую (воды нет -> нечего поднимать -> лишняя трата энергии и ресурса насоса). Для этого нужно подождать некоторое время, чтобы вода накопилась. Обычно это занимает около 5 минут. В течение этого времени насос работать не должен.

Однако работать долго он тоже не должен ибо вода может кончится и он опять будет работать в холостую. Ждать он долго тоже не должен (не больше 5 минут) ибо это будет просто тратой времени.

Чтобы решить проблему, первое что пришло в голову - это купить готовый таймер. Пошел в мегастрой и нашел этот (типичное решение из магазина)

![as](https://avatars.mds.yandex.net/get-mpic/1750349/img_id6228458550028284933.jpeg/orig)

Но данный таймер (и его альтернативы) обладают рядом недостатков:
- Они не имеют защитного корпуса (работать придется на улице)
- Не имеют дисплея (смотреть будет пожилой человек)
- ~~Тупые~~ Не умные. Нельзя настраивать время простоя и время работы по отдельности (Это ведет к работе в холостую)
- Минимальный шаг - 15 минут (работа в холостую)

В итоге было решено ради интереса и ради практики сделать свое решение на базе платформы Arduino. Выбор был сделан достаточно быстро. Это **Arduino Nano**.

### Железо

- В основе лежит Arduino Nano (купленого с известного сайта)
- Корпус взят от нерабочего счетчика "Меркурий".
- Плата взята от того же счетчика. Были выпаяны все ненужные детали, а на их место припаяны нужные.
- Семисегментный индикатор (4 цифры)

![Fritzing](https://i.imgur.com/tlMhouo.png)

### Софт

- **PlatformIO** - плагин для Visual Studio Code
- **AlexButton** - библиотека для обработки нажатий кнопки (разные методы нажатия, устранение дрибезжания и т.п.)
- **EEPROM** - стандартная библиотека для работы со встроенной памяти Arduino. Это нужно, чтобы запоминать настройки времени после отключения устройства
- **Ticker** - библиотека для работы с таймером
- **Fritzing** - среда проектирования схем Arduino

### Функциональность

- Показ времени работы - первые две цифры. Показ времени простоя - последние 2 цифры
- Отсчет секунд с помощью моргания точки по середине
- Изменение времени таймеров
- Показывание актуальных настроек
- Запись настроек
- Сохранение настроек после перезапуска
- Клеммы для входа 220 вольт. Клеммы для выхода 220 в
- Влагозащитный, износоустойчивый корпус.

### Код проекта

Главный файл - **/src/main.ino**

### Фоторепортаж
![1](https://i.imgur.com/44Qf9qH.jpg)
![2](https://i.imgur.com/59j7GTK.jpg)
![3](https://i.imgur.com/UpgnU1Q.jpg?1)
