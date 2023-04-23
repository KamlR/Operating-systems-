# Программа на 4 балла:

## Выполнено Мавлетовой Кариной Радиковной, БПИ216, Вариант 11
### Условие задачи:
*Разработка программы, осуществляющей взаимодействие между тремя дочерними процессами с использованием неименованных каналов.*
 
 *Разработать программу вычисления отдельно количества гласных и согласных букв в ASCII-строке.*


#### :white_check_mark: Представлена общая схема решаемой задачи:
Если словами - у меня есть родительский процесс, который считывает строку из файла (имя файла передаётся через командную строку). Затем родительский процесс, используя pipe, передаёт данные child процессу (создан при помощи fork). Child процесс в свою очередь читает строку от parent процесса при помощи pipe, подсчитывает в ней количество гласных и согласных. На этом взаимодействие parent и child процесса заканчивается. Теперь child процесс порождает grandson процесс при помощи fork. Для них создаётся pipe для обмена информацией. Child процесс передаёт через pipe grandson процессу информацию о кол-ве гласных и согласных. Grandson читает это из pipe и записывает в файл (имя файла для записи было передано через командную строку).

А теперь это взаимодействие картинкой:

<img width="721" alt="image" src="https://user-images.githubusercontent.com/115434090/224700188-1eeacc54-66b3-4567-b122-928f0f980b53.png">

#### :white_check_mark: Разработано консольное приложение, использующее неименованные каналы, обеспечивающее взаимодействие по следующей схеме:
+ первый процесс читает текстовые данные из заданного файла и через неименованный канал передает их второму процессу;
+ второй процесс осуществляет обработку данных в соответствии с заданием и передает результат обработки через неименованный канал третьему процессу;
+ третий процесс осуществляет вывод данных в заданный файл.
Именно эта схема подробно описана выше конкретно для моей задачи.

#### :white_check_mark: Для задания имен входного и выходного файлов используются аргументы командной строки.
Да, вторым параметром - входной файл, третьим - выходной файл.

#### :white_check_mark: Ввод и вывод данных при работе с файлами осуществляется через системные вызовы read и write.
Чтение и запись обычных файлов организована через read, write. Работа с pipe тоже.

#### :white_check_mark: Размеры буферов для хранения вводимых данных и результатов обработки должны быть не менее 5000 байт. Допускается работа только с такими файлам, которые за одно заносятся в буфер за одно чтение. То есть допускается работа с файлом ограниченного размера. Это позволяет не накладывать дополнительных условий на алгоритмы обработки данных.

Строки, в которых хранятся данные могут вместить в себя максимально 5000 байт. Соответственно при передачи данных через pipe получится поместить туда максимально 5000 байт, считать аналогично.

#### :white_check_mark: Представлен набор (не менее пяти) текстовых файлов, на которых проводилось тестирование программы и файлы с результатом.
В папке tests хранятся все данные тестов. В формате 1_in.txt, 1_out.txt и так далее до пяти.

