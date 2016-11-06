# Введение

Проект Мяука представляет собой генератор лексических анализаторов, порождающий текст лексического анализатора на языке C++. К данному моменту имеется достаточно много таких генераторов, например [Coco/R](http://www.ssw.uni-linz.ac.at/Coco),
[flex](http://flex.sourceforge.net), [flex++](http://www.mario-konrad.ch/wiki/doku.php?id=programming:flexbison:flexppbisonpp), [flexc++](http://flexcpp.sourceforge.net), и этот список далеко не полон. Однако у всех этих генераторов есть один общий недостаток. Состоит указанный недостаток в том, что этими генераторами, по существу, автоматизируются лишь задачи проверки корректности записи и обнаружения начала лексем, а порождение значения лексемы по её строковому представлению должно выполняться вызываемой после проверки корректности лексемы функцией, написанной пользователем генератора. При этом, во–первых, дважды выполняется проход по фрагменту входной строки, и, во–вторых, приходится вручную реализовывать часть конечного автомата, построенного генератором лексических анализаторов. Предлагаемый генератор нацелен на устранение данного недостатка.

# Формат входного файла

Входной файл с описанием лексического анализатора состоит из последовательности следующих команд (из них обязательна только команда %codes), которые могут идти в произвольном порядке:  
%scaner\_name имя\_сканера  
%codes\_type имя\_типа\_кодов\_лексемы  
%ident\_name имя\_идентификатора  
%token\_fields добавляемые\_поля\_лексемы  
%class\_members добавляемые\_члены\_класса  
%newline\_is\_lexem  
%codes имя\_кода\_лексемы {, имя\_кода\_лексемы}  
%keywords \[действия\_по\_завершении:\] строка\_представляющая\_ключевое\_слово : код\_ключевого\_слова {, строка\_представляющая\_ключевое\_слово : код\_ключевого\_слова}  
%delimiters \[действия\_по\_завершении:\] строка\_представляющая\_разделитель\_слово : код\_разделителя {, строка\_представляющая\_разделитель : код\_разделителя}  
%idents '{'описание\_начала\_идентификатора'}' '{'описание\_тела\_идентификатора'}'  
%numbers \[действия\_при\_инициализации\]:\[действия\_по\_завершении\] {%action имя\_действия определение\_действия} '{'выражение'}'  
%strings \[действия\_при\_инициализации\]:\[действия\_по\_завершении\] {%action имя\_действия определение\_действия} '{'выражение'}'  
%comments \[%single\_lined начало\_однострочного комментария\] \[%multilined \[%nested] начало\_многострочного комментария : конец\_многострочного\_комментария\]

Прежде чем пояснить смысл каждой из приведённых выше конструкций, условимся, что всё, заключённое в квадратные скобки, является необязательным, а всё, заключённое в фигурные скобки может повторяться любое число  раз, в том числе и ни разу. При этом '{' и '}' обозначают сами фигурные скобки.


