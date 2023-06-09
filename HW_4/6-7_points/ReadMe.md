# Мавлетова Карина Радиковна, БПИ216, Вариант 15
## 6–7 баллов 
### В дополнение к предыдущей программе необходимо разработать клиентскую программу, подключаемую к серверу, которая предназначена для отображения комплексной информации о выполнении приложения в целом. То есть, данный программный модуль должен адекватно отображать поведение моделируемой системы, позволяя не пользоваться отдельными видами, предоставляемыми клиентами и серверами по отдельности.

**Условие задачи:**

*Задача о клумбе – 1. На клумбе растет 40 цветов, за ними непрерывно следят два процесса–садовника и поливают увядшие цветы, при этом оба садовника очень боятся полить один и тот же
цветок, который еще не начал вянуть. Создать приложение,
моделирующее состояния цветков на клумбе и действия
садовников. Сервер используется для изменения состояния цветов, каждый садовник — отдельный клиент.*

### Про файлы:
- В папке code:
    - server.c - код сервера, который решает, какие цветы поливать
    - client1.c - код первого клиента (садовника)
    - client2.c - код второго клиента (садовника)
    - info.c - отдельный вид клиента, который был добавлен на этом этапе и который выводит комплексную информацию о работе приложения.
    - ссылка на видео с демонстрацией работы программы: https://disk.yandex.ru/i/clGceo-hweAKaQ
### Как запускать:
Для начала запускается сервер. Далее он сообщает о том, что ждёт клиентов для подключения. Потом запускается первый сад, за ним второй. После них подключается info клиент, который будет отображать всю информацию о работе приложения.

### Критерии:

#### :white_check_mark: Представлен сценарий решаемой задачи:
В моей задаче в качестве сервера выступает сад. Сервер изменяет состояние цветов, т.е решает, какие из них следует полить в очередной день. А вот клиенты - два садовника, которые занимаются поливкой.
Всего есть пять дней полива. В каждый из пяти дней сервер рандомно выбирает цветы, которые нужно поливать. Цветы для полива обозначаются 0. После сервер посылает данные о поливке
цветов первому клиенту (отправляется массив char по udp). Он в свою очередь запоминает номера цветов, которые полил в массив int. Данный массив направляется серверу. Со вторым клиентом (садовником такая же история). Когда сервер получает от клиентов номера политых цветов - он отправлялет массив int с этими номерами info клиенту. Он уже отображает данную информацию. Соотв. info клиент располагает информацией о поливке цветов, как 1 садовником, так и 2. Также info клиент выводит информацию о дне полива и об окончании полива очередным садовником. Таким образом и складывается комплексная информация, которой обладает info клиент.

#### :white_check_mark: При запуске программ требуемые для их работы IP адреса и порты необходимо задавать в командной строке, чтобы обеспечить гибкую подстройку к любой сети:
- сервер получает через аргументы командной строки номер порта
- 1 клиент получает через аргументы командной строки номер порта и ip
- 2 клиент получает через аргументы командной строки номер порта и ip
- 3 клиент получает через аргументы командной строки номер порта и ip

#### :white_check_mark: Разработанное приложение должно работать как на одном компьютере так и в распределенном (сетевом) режиме на нескольких компьютерах, по которым можно будет разнести серверы и клиентов.

#### :white_check_mark: Завершение работы клиентов и серверов на данном этапе не оговаривается. Но оно должно быть представлено в сценарии:
Клиенты и сервер заканчивают свою работу после 5 дней полива.


