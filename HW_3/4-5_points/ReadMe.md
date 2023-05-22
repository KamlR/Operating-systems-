# Мавлетова Карина Радиковна, БПИ216, Вариант 15
## 4–5 баллов 
### Разработать клиент–серверное приложение, в котором сервер (или серверы) и клиенты независимо друг от друга отображают только ту информацию, которая поступает им во время обмена. То есть, отсутствует какой-либо общий вывод интегрированной информации, отображающий поведение системы в целом.

**Условие задачи:**

*Задача о клумбе – 1. На клумбе растет 40 цветов, за ними непрерывно следят два процесса–садовника и поливают увядшие цветы, при этом оба садовника очень боятся полить один и тот же
цветок, который еще не начал вянуть. Создать приложение,
моделирующее состояния цветков на клумбе и действия
садовников. Сервер используется для изменения состояния цветов, каждый садовник — отдельный клиент.*

### Про файлы:
- файл server.c содержит код сервера, который отвечает за выбор цветов для поливки
- файл client1.c содержит код первого клиента, который обращается к серверу (первый садовник)
- файл client2.c содержит код второго клиента, который обращается к серверу (второй садовник)
- также для каждого из вышеуказанных файлов имеются соотв. исполняемые файлы
- в папке tests содеражатся файлы .txt, которые иллюстрируют вывод каждого участника сада
### Как запускать:
Для начала запускается сервер. Далее он сообщает о том, что ждёт клиентов для подключения. Потом запускается первый сад, за ним второй. Они подключаются к серверу и начинается обмен информацией.

### Критерии:

#### :white_check_mark: Представлен сценарий решаемой задачи:
В моей задаче в качестве сервера выстпупает сад. Сервер изменяет состояние цветов, т.е решает, какие из них следует полить в очередной день. А вот клиенты - два садовника, которые занимаются поливокй.
Всего есть пять дней полива. В каждый из пяти дней сервер рандомно выбирает цветы, которые нужно поливать. Цветы для полива обозначаются 0. После сервер посылает данные о поливке
цветов первому клиенту (отправляется массив char по tcp). Первый садовник принимает данные, далее выводит информацию о поливке каждого цветка. Когда первый садовни завершает полив, то направляет
серверу данные о том, что полив 1 садовником в такой то день завершён. Как только получено данное сообщение - сервер отправляет на поливку второго садовника. Со 2 садовником всё аналогично, как и с 1.
Важно отметить, что 1 садовник может поливать цветы с 1 по 20, а второй с 21 по 40.

#### :white_check_mark: При запуске программ требуемые для их работы IP адреса и порты необходимо задавать в командной строке, чтобы обеспечить гибкую подстройку к любой сети:
- сервер получает через аргументы командной строки номер порта
- 1 клиент получает через аргументы командной строки номер порта и ip
- 2 клиент получает через аргументы командной строки номер порта и ip

#### :white_check_mark: Разработанное приложение должно работать как на одном компьютере так и в распределенном (сетевом) режиме на нескольких компьютерах, по которым можно будет разнести серверы и клиентов.

#### :white_check_mark: Завершение работы клиентов и серверов на данном этапе не оговаривается. Но оно должно быть представлено в сценарии:
Клиенты и сервер заканчивают свою работу после 5 дней полива.

