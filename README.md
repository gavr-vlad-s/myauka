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
%comments \[%single\_lined начало\_однострочного комментария\] \[%multilined \[%nested] начало\_многострочного\_комментария : конец\_многострочного\_комментария\]

Прежде чем пояснить смысл каждой из приведённых выше конструкций, условимся, что всё, заключённое в квадратные скобки, является необязательным, а всё, заключённое в фигурные скобки может повторяться любое число  раз, в том числе и ни разу. При этом '{' и '}' обозначают сами фигурные скобки.

Далее отметим, что под строковым литералом Мяуки (далее просто строковым литералом) будет пониматься любая (в том числе пустая) цепочка символов, заключённая в двойные кавычки. Если в этой последовательности нужно указать саму двойную кавычку, то эту кавычку нужно удвоить.  

Перейдём теперь к пояснению команд, описывающих лексический анализатор (далее, для краткости, — сканер).  

Прежде всего, если указана команда  
>%scaner_name имя\_сканера  

то в одном из заголовочных файлов появляется запись вида  

```c++
class имя\_сканера {
    ...
};  
```

Сам же этот заголовочный файл будет называться имя\_сканера'.h. Соответствующий файл реализации будет называться имя\_сканера'.cpp, где имя\_сканера' — имя\_сканера, преобразованное к нижнему регистру. Принятое по умолчанию имя\_сканера — Scaner.  

Далее, если указана команда  

>%codes_type имя\_типа\_кодов\_лексем  

то команда %codes порождает в файле имя\_сканера'.h запись вида 

```c++
enum имя_типа_кодов_лексем : unsigned short {  
    NONE,  
    UNKNOWN,  
    имя_кода_лексемы1,  
    ...  
    имя_кода_лексемыN  
};
```
где имя\_кода\_лексемы1, ..., имя\_кода\_лексемыN -  имена кодов лексем, определённые в разделе %codes. Принятое по умолчанию имя типа кодов лексем - Lexem_code.  

Команда  

> %ident\_name имя\_идентификатора  

указывает имя кода лексемы для лексемы 'идентификатор'. Если в языке, для которого пишется сканер, идентификаторов нет, то команда %ident_name необязательна.  

Если в описание лексемы нужно добавить какие-либо поля, то необходимо написать команду  

>%token\_fields добавляемые\_поля\_лексемы  

где добавляемые\_поля\_лексемы - строковый литерал с описанием нужных полей. Например, если лексема может принимать как значения типа \_\_float128, так и значения типа \_\_int128, причём поле типа \_\_float128} по условиям задачи нужно назвать x, а поле типа  \_\_int128 - y, то строковый литерал с добавляемыми в лексему полями может выглядеть, например, так:  

>"\_\_float128 x;  
>\_\_int128    y;"  

Кроме того, если в класс сканера требуется добавить члены, необходимые для каких-либо вычислений, то нужно написать  

%class_members добавляемые\_члены\_класса  

где под добавляемые\_члены\_класса понимается строковый литерал, содержащий перечень добавляемых в сканер членов. Если, например, нужно добавить 

>\_\_int128   integer\_value;  
>\_\_float128 integer\_part;  
>\_\_float128 fractional\_part;  
>\_\_float128 exponent;  

то вместо добавляемые\_члены\_класса  

>"\_\_int128   integer\_value;  
>\_\_float128 integer\_part;  
>\_\_float128 fractional\_part;  
>\_\_float128 exponent;"  

Если необходимо, чтобы символ '\\n' перехода на новую строку был отдельной лексемой, а не пробельным символом, то нужно указать команду 
>%newline_is_lexem  

В обязательном разделе  %codes содержится список разделённых запятыми идентификаторов (правила построения идентификаторов -  такие же, что и в C++), представляющих собой имена кодов лексем. Например, если имя перечисления с кодами лексемы не указано командой %codes\_type}, и раздел %codes имеет вид  

>%codes  
>   Kw_if, Kw_then, Kw_else, Kw_endif  

то будет порождено перечисление  

```c++
enum Lexem_code : unsigned short {  
    NONE,    UNKNOWN,  
    Kw_if,   Kw_then,   
    Kw_else, Kw_endif  
%};
```  

Иными словами, всегда определяется два специальных кода лексем: NONE, обозначающее конец обрабатываемого текста, и UNKNOWN, обозначающее неизвестную лексему.  

В разделе %keywords} указываются ключевые слова языка, для которого пишется сканер, и соответствующие этим ключевым словам коды  лексем, взятые из раздела %codes. Например, если имеются ключевые слова __if__, __then__, __else__, __endif__, и этим ключевым словам соответствуют коды лексем Kw\_if, Kw\_then, Kw\_else, Kw\_endif, то раздел %keywords должен иметь следующий вид:  

   %keywords  
       ...  
       "if" : Kw_if,  
       "then" : Kw_then,  
       "else" : Kw_else,  
       "endif" : Kw_endif  
       ...  

Здесь многоточием обозначено (возможно, имеющееся) описание других ключевых слов.  

В разделе %idents определяется структура идентификатора того языка, для которого пишется сканер. Более точно, описание\_начала\_идентификатора указывает, что может быть в начале идентификатора, а описание\_тела\_идентификатора - как устроено тело идентификатора.  

В разделе %delimiters указываются разделители и знаки операций языка, для которого пишется сканер, и соответствующие этим разделителям и знакам операций коды лексем, взятые из раздела %codes. Например, если в языке есть разделители \<, \>, \<=, \>=, =, !=, с соответствующими кодами лексем del\_LT, del\_GT, del\_LEQ, del\_GEQ, del\_EQ, del\_NEQ, то раздел %delimiters должен иметь вид  

   %delimiters  
       ...  
       "<"  : del_LT,  
       ">"  : del_GT,  
       "<=" : del_LEQ,  
       ">=" : del_GEQ,  
       "="  : del_EQ,  
       "!=" : del_NEQ  
       ...  

Здесь многоточием обозначено (возможно, имеющееся) описание других разделителей и знаков операций.  

В разделе %numbers указывается регулярное выражение, определяющее числа, с внедрёнными в это регулярное выражение действиями. Каждое из действий должно быть описано командой  

>%action имя\_действия определение\_действия  

где имя\_действия - идентификатор языка C++, являющийся именем определяемого действия, а определение\_действия -
строковый литерал, содержащий код на C++, выполняющий нужное действие.  

В разделе %strings описывается структура строковых и символьных литералов (если символьные литералы вообще есть) языка, для которого пишется сканер. Раздел %strings устроен так же, как и раздел %numbers. При этом при указании раздела %strings} у класса сканера автоматически определяются члены std::string\ buffer и int char\_code}.  

Наконец, в разделе \textbf{\%comments} описывается структура комментариев языка, для которого пишется сканер.  

Командой   

>%single_lined начало\_однострочного\_комментария  

где начало\_однострочного\_комментария - строковый литерал, представляющий цепочку символов, являющуюся началом однострочного комментария, определяется структура однострочного комментария.  

Командой же  

>%multilined [%nested] начало\_многострочного\_комментария : конец\_многострочного\_комментария  

определяется структура многострочного комментария. А именно, начало\_многострочного\_комментария и конец\_многострочного\_комментария - строковые литералы, являющиеся цепочками символов, начинающих и заканчивающих многострочный комментарий. Если указано слово %nested, то многострочный комментарий может быть вложенным.  

Поясним теперь, что в Мяуке понимается под началом идентификатора, концом идентификатора, и регулярным выражением:  

%\textcolor{Green}{\synt{описание_начала_идентификатора} $\to$ }\textcolor{Green}{\synt{выр}}
%
%\textcolor{Green}{\synt{описание_тела_идентификатора} $\to$ }\textcolor{Green}{\synt{выр}}
%
%\textcolor{Green}{\synt{выр} $\to$ \synt{выр}$_0$\texttt{\{}\textcolor{Black}{|}\synt{выр}$_0$\texttt{\}}}
%
%\textcolor{Green}{\synt{выр}$_0$ $\to$ \synt{выр}$_1$\texttt{\{}\synt{выр}$_1$\texttt{\}}}
%
%%\textcolor{Green}{\synt{выр}$_1$ $\to$ \synt{выр}$_2$\texttt{[}\textcolor{Black}{?}|\textcolor{Black}{*}|\textcolor{Black}{+}\texttt{]}}
%
%\textcolor{Green}{\synt{выр}$_1$ $\to$ \textcolor{Black}{символ} | \synt{класс_символов}}
%
%\textcolor{Green}{\synt{класс_символов} $\to$ \texttt{\textcolor{Black}{[:Latin:]}} | \texttt{\textcolor{Black}{[:latin:]}} | \texttt{\textcolor{Black}{[:Russian:]}} |
%\texttt{\textcolor{Black}{[:russian:]}} | \texttt{\textcolor{Black}{[:bdigits:]}} |\\
%{}\texttt{\textcolor{Black}{[:odigits:]}} | \texttt{\textcolor{Black}{[:digits:]}} | \texttt{\textcolor{Black}{[:xdigits:]}} | \texttt{\textcolor{Black}{[:Letter:]}} |
%\texttt{\textcolor{Black}{[:letter:]}} | \texttt{\textcolor{Black}{[:nsq:]}} | \texttt{\textcolor{Black}{[:ndq:]}} }
%
%\textcolor{Green}{\synt{выражение} $\to$ \synt{выражение}$_0$ \texttt{\{}\textcolor{Black}{|}\synt{выражение}$_0$ \texttt{\}}
%}
%
%\textcolor{Green}{\synt{выражение}$_0$ $\to$ \synt{выражение}$_1$ \texttt{\{}\synt{выражение}$_1$\texttt{\}}
%}
%
%\textcolor{Green}{\synt{выражение}$_1$ $\to$ \synt{выражение}$_2$\texttt{[}\textcolor{Black}{?}|\textcolor{Black}{*}|\textcolor{Black}{+}\texttt{]} }
%
%\textcolor{Green}{\synt{выражение}$_2$ $\to$ \synt{выражение}$_3$\texttt{[}\textcolor{Black}{\$}\synt{имя_действия}\texttt{]} }
%
%\textcolor{Green}{\synt{выражение}$_3$ $\to$ \textcolor{Black}{символ} | \synt{класс_символов} | \textcolor{Black}{(}\synt{выражение}\textcolor{Black}{)}}
%
%
%В этой грамматике под словом \glqq символ\grqq\ понимается следующее. Любой непробельный символ, кроме символов \texttt{'|'}, \texttt{'*'}, \texttt{'+'}, \texttt{'?'},
%'\texttt{\$'}, \texttt{'\textbackslash'}, \texttt{'"{}'}, и символа перехода на новую строку, в файле с описанием сканера представляет самого себя. Если же эти символы нужно
%указать в регулярном выражении, то следует их записывать как \texttt{'\textbackslash|'}, \texttt{'\textbackslash*'}, \texttt{'\textbackslash+'}, \texttt{'\textbackslash?'},
%'\texttt{\textbackslash\$'}, \texttt{'\textbackslash\textbackslash'}, \texttt{'\textbackslash"{}'}, \texttt{'\textbackslash{}n'} соответственно. При этом все пробельные 
%символы (то есть символы, коды которых не превосходят кода пробела) генератором лексических анализаторов Мяука игнорируются.
%
%В приводимой ниже таблице поясняются допускаемые классы символов.\newpage
%
%\begin{table}[!h]
%\centering
%\caption{Допускаемые классы символов.}
%\vspace{1mm}
%\begin{tabular}{|l|p{125mm}|}\hline 
%Класс символов                          & Пояснение                                                                               \\ \hline
%\texttt{\textcolor{Black}{[:Latin:]}}   & Прописные латинские буквы от 'A' до 'Z'.                                                \\ \hline 
%\texttt{\textcolor{Black}{[:latin:]}}   & Строчные латинские буквы от 'a' до 'z'.                                                 \\ \hline 
%\texttt{\textcolor{Black}{[:Russian:]}} & Прописные русские буквы от 'А' до 'Я' (включая букву 'Ё').                              \\ \hline
%\texttt{\textcolor{Black}{[:russian:]}} & Строчные русские буквы от 'а' до 'я' (включая букву 'ё').                               \\ \hline
%\texttt{\textcolor{Black}{[:bdigits:]}} & Символы двоичных цифр, т.е. символы '0' и '1'.                                          \\ \hline
%\texttt{\textcolor{Black}{[:odigits:]}} & Символы восьмеричных цифр, т.е. символы '0', '1', '2', '3', '4', '5', '6', '7'.         \\ \hline
%\texttt{\textcolor{Black}{[:digits:]}}  & Символы десятичных цифр, т.е. символы '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'. \\ \hline
%\texttt{\textcolor{Black}{[:xdigits:]}} & Символы шестнадцатеричных цифр, т.е. символы '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'a', 'b', 'c', 'd', 'e', 'f'.\\ \hline
%\texttt{\textcolor{Black}{[:Letter:]}}  & Прописные латинские буквы от 'A' до 'Z' и прописные русские буквы от 'А' до 'Я' (включая букву 'Ё').  \\ \hline
%\texttt{\textcolor{Black}{[:letter:]}}  & Строчные латинские буквы от 'a' до 'z' и строчные русские буквы от 'а' до 'я' (включая букву 'ё').    \\ \hline
%\texttt{\textcolor{Black}{[:nsq:]}}     & Символы, отличные от одинарной кавычки (').\\ \hline
%\texttt{\textcolor{Black}{[:ndq:]}}     & Символы, отличные от двойной кавычки (").\\ \hline
%\end{tabular} 
%\end{table}
%
%Из всех этих классов символов классы \texttt{\textcolor{Black}{[:nsq:]}} и \texttt{\textcolor{Black}{[:ndq:]}} допускаются только в разделе \textbf{\%strings}.






