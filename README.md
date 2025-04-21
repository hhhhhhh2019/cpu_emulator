
# Table of Contents

1.  [](#org7ef2579)
2.  [Регистры](#orgfc76063)
    1.  [Состояния ядра](#org8206b3d)
3.  [Ядро](#org35d510a)
    1.  [Шины](#org45faf29)
    2.  [Микрокоды](#org27a5348)
    3.  [Инструкции](#org70ff089)
4.  [Прерывания](#org74aa681)


<a id="org7ef2579"></a>

# TODO 

-   [ ] SIMD
-   [X] полное и занковое(или как это по-русски называется) умножение
    решил, что не нужно
-   [ ] деление со знаком
-   [X] stou, loau
    -   [X] инструкции для переноса значений из user r.f. в kernel r.f. и наоборот
    -   [X] нужны ли тогда stou и loau?
-   [ ] понять, нужны ли прерывания в kernel mode
    да, нужны, так как подтверждение чтения и записи я собираюсь делать через прерывания, а ядру при инициализации нужно читать таблицу разделов на диске
    но в таком случае нужно как-то делать возврат из прерывания в kernel mode, что можно сделать через сохранение pc в стек, но именно этого я пытался избежать используя два регистровых файла
    как вариант, можно сохранать pc в стек только в том случае, если флаг USERMODE выключен
-   [ ] при чтении 8 битного отриц. числа в 64 битное надо сохранить знак
    а зачем, если загружать два 8-и битных числа, например перемножать их, и сохранать результат опять в 8 бит, то нам совершенно безразницы, что просходит битах старше 8
    вопрос только в делении
-   [ ] MMIO
-   [ ] аппаратные прерываня
-   [ ] виртуальная память
-   [ ] syscall
-   [ ] рассмотреть возможность перехода на размер маш. слова в 12 байт с выравниванием в 4 байта


<a id="orgfc76063"></a>

# Регистры

Регистровый файл имеет интерестую особенность: он умеет в двойное продолжительное чтение.
То есть можно одновременно считывать два регистра(или один и тот же, но зачем) и параллельно чтению
производить запись. При этом запись в регистр, который сейчас читается, никак не скажется на выходе.
Это позволяет очень сильно упростить реализацию микрокодов.

всего есть 18 регистров:

-   0 - всегда 0
-   1..14 - регистры общего назначения
-   15 - указатель на текущую инструкцию
-   16 - указатель на таблицу виртуальных адресов(см. ниже)
-   17 - регистр флагов

К 16 и 17 регистру обращение происходит через специальные инструкции.

все эти регистры содержатся в 2 экземлярах: для kernel mode и user mode

за переключение между этими режимами отвечает флаг в регистре состояний ядра(см. ниже)


<a id="org8206b3d"></a>

## Состояния ядра

<table border="2" cellspacing="0" cellpadding="6" rules="groups" frame="hsides">


<colgroup>
<col  class="org-right" />

<col  class="org-left" />

<col  class="org-left" />
</colgroup>
<thead>
<tr>
<th scope="col" class="org-right">№ бита</th>
<th scope="col" class="org-left">Имя</th>
<th scope="col" class="org-left">Значение</th>
</tr>
</thead>
<tbody>
<tr>
<td class="org-right">0</td>
<td class="org-left">ENABLED</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">1</td>
<td class="org-left">INTERRUPTS</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">2</td>
<td class="org-left">PAGING</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">3</td>
<td class="org-left">USER MODE</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">4</td>
<td class="org-left">ISINTERRUPT</td>
<td class="org-left">включается автоматически. если 1, то USER MODE = 0 и остальные прерывания не вызваются</td>
</tr>
</tbody>
</table>


<a id="org35d510a"></a>

# Ядро


<a id="org45faf29"></a>

## Шины

-   sdb (shared data bus)
-   ab (address bus)
-   Rout1 (reg. file output 1)
-   Rout2 (reg. file output 2)


<a id="org27a5348"></a>

## Микрокоды

как работают матем. инструкции:

1.  считать r2
2.  считать r3
3.  обработать их(+,-,\*,/,&&#x2026;)
4.  sdb -> r1

как работают матем. инструкции:

1.  считать r2
2.  считать num64
3.  обработать их(+,-,\*,/,&&#x2026;)
4.  sdb -> r1

как работает addz:

1.  is<sub>zero</sub>
2.  считать r2
3.  считать num64
4.  обработать их(+,-,\*,/,&&#x2026;)
5.  sdb -> r1

как работает addc:

1.  is<sub>carry</sub>
2.  считать r2
3.  считать num64
4.  обработать их(+,-,\*,/,&&#x2026;)
5.  sdb -> r1

как работает adds:

1.  is<sub>sign</sub>
2.  считать r2
3.  считать num64
4.  обработать их(+,-,\*,/,&&#x2026;)
5.  sdb -> r1

как работают инстр. для записи:

1.  считать r2
2.  считать num64
3.  сложить их
4.  sdb -> ab
5.  сброс шин(кроме ab)
6.  считать r3 -> sdb
7.  запись

как работают инстр. для чтения:

1.  считать r2
2.  считать num64
3.  сложить их
4.  sdb -> ab
5.  сброс шин(кроме ab)
6.  чтение
7.  sdb -> r1

push:

1.  считать sp
2.  сложить с 0(или я сделаю перенос с r1 в sdb)
3.  sdb -> ab(или я сделаю r1 -> ab)
4.  сброс шин(кроме ab)
5.  считать r3 -> sdb
6.  запись
7.  уменьшение sp

pop:

1.  считать sp
2.  сложить с 0(или я сделаю перенос с r1 в sdb)
3.  sdb -> ab(или я сделаю r1 -> ab)
4.  сброс шин(кроме ab)
5.  чтение
6.  sdb -> r1
7.  увеличение sp

call:

1.  считать sp
2.  сложить с 0(или я сделаю перенос с r1 в sdb)
3.  sdb -> ab(или я сделаю r1 -> ab)
4.  сброс шин(кроме ab)
5.  считать pc -> sdb
6.  запись
7.  увеличение sp
8.  r3 -> pc

int:

1.  num8 -> core<sub>int</sub>

iret:

1.  ISINTERRUPT off

chst:

1.  проверка USERMODE
2.  считать r2
3.  сложить с 0(или я сделаю перенос с r1 в sdb)
4.  sdb -> state

lost:

1.  проверка USERMODE
2.  state -> sdb
3.  sdb -> r1

chtp:

1.  проверка USERMODE
2.  считать r2
3.  сложить с 0(или я сделаю перенос с r1 в sdb)
4.  sdb -> tp

lotp:

1.  проверка USERMODE
2.  tp -> sdb
3.  sdb -> r1

chflag:

1.  проверка USERMODE
2.  считать r2
3.  сложить с 0(или я сделаю перенос с r1 в sdb)
4.  sdb -> flag

loflag:

1.  проверка USERMODE
2.  flag -> sdb
3.  sdb -> r1

utok:

1.  проверка USERMODE
2.  считать r3<sub>u</sub> -> sdb
3.  sdb -> r1

ktou:

1.  проверка USERMODE
2.  считать r3 -> sdb
3.  sdb -> r1<sub>u</sub>

<table border="2" cellspacing="0" cellpadding="6" rules="groups" frame="hsides">


<colgroup>
<col  class="org-left" />

<col  class="org-left" />
</colgroup>
<thead>
<tr>
<th scope="col" class="org-left">Имя</th>
<th scope="col" class="org-left">Описание</th>
</tr>
</thead>
<tbody>
<tr>
<td class="org-left">inter<sub>off</sub></td>
<td class="org-left">ISINTERRUPT off</td>
</tr>

<tr>
<td class="org-left">num8<sub>to</sub><sub>core</sub><sub>int</sub></td>
<td class="org-left">core<sub>int</sub>(num8)</td>
</tr>

<tr>
<td class="org-left">pc<sub>to</sub><sub>sdb</sub></td>
<td class="org-left">pc -&gt; sdb</td>
</tr>

<tr>
<td class="org-left">r3<sub>to</sub><sub>pc</sub></td>
<td class="org-left">r3 -&gt; pc</td>
</tr>

<tr>
<td class="org-left">sdb<sub>to</sub><sub>ab</sub></td>
<td class="org-left">sdb -&gt; ab</td>
</tr>

<tr>
<td class="org-left">sdb<sub>to</sub><sub>flag</sub></td>
<td class="org-left">sdb -&gt; flag</td>
</tr>

<tr>
<td class="org-left">sdb<sub>to</sub><sub>r1</sub></td>
<td class="org-left">sdb -&gt; r1</td>
</tr>

<tr>
<td class="org-left">sdb<sub>to</sub><sub>r1</sub><sub>u</sub></td>
<td class="org-left">sdb -&gt; r1<sub>u</sub></td>
</tr>

<tr>
<td class="org-left">sdb<sub>to</sub><sub>state</sub></td>
<td class="org-left">sdb -&gt; state</td>
</tr>

<tr>
<td class="org-left">sdb<sub>to</sub><sub>tp</sub></td>
<td class="org-left">sdb -&gt; tp</td>
</tr>

<tr>
<td class="org-left">state<sub>to</sub><sub>sdb</sub></td>
<td class="org-left">state -&gt; sdb</td>
</tr>

<tr>
<td class="org-left">tp<sub>to</sub><sub>sdb</sub></td>
<td class="org-left">tp -&gt; sdb</td>
</tr>

<tr>
<td class="org-left">flag<sub>to</sub><sub>sdb</sub></td>
<td class="org-left">flag -&gt; sdb</td>
</tr>

<tr>
<td class="org-left">write</td>
<td class="org-left">запись</td>
</tr>

<tr>
<td class="org-left">read</td>
<td class="org-left">чтение</td>
</tr>

<tr>
<td class="org-left">is<sub>usermode</sub></td>
<td class="org-left">проверка USERMODE</td>
</tr>

<tr>
<td class="org-left">is<sub>zero</sub></td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-left">is<sub>carry</sub></td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-left">is<sub>sign</sub></td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-left">bus<sub>reset</sub></td>
<td class="org-left">сброс шин(кроме ab)</td>
</tr>

<tr>
<td class="org-left">read<sub>num64</sub></td>
<td class="org-left">считать num64</td>
</tr>

<tr>
<td class="org-left">read<sub>r2</sub></td>
<td class="org-left">считать r2</td>
</tr>

<tr>
<td class="org-left">read<sub>r3</sub></td>
<td class="org-left">считать r3</td>
</tr>

<tr>
<td class="org-left">r3<sub>to</sub><sub>sdb</sub></td>
<td class="org-left">считать r3 -&gt; sdb</td>
</tr>

<tr>
<td class="org-left">r3<sub>u</sub><sub>to</sub><sub>sdb</sub></td>
<td class="org-left">считать r3<sub>u</sub> -&gt; sdb</td>
</tr>

<tr>
<td class="org-left">read<sub>sp</sub></td>
<td class="org-left">считать sp</td>
</tr>

<tr>
<td class="org-left">inc<sub>sp</sub></td>
<td class="org-left">увеличение sp</td>
</tr>

<tr>
<td class="org-left">dec<sub>sp</sub></td>
<td class="org-left">уменьшение sp</td>
</tr>

<tr>
<td class="org-left">ALU<sub>sum</sub></td>
<td class="org-left">+</td>
</tr>

<tr>
<td class="org-left">ALU<sub>sub</sub></td>
<td class="org-left">-</td>
</tr>

<tr>
<td class="org-left">&#x2026;</td>
<td class="org-left">&#xa0;</td>
</tr>
</tbody>
</table>


<a id="org70ff089"></a>

## Инструкции

структура инструкции:

-   0..7 - opcode
-   8..11 - register 1
-   12..15 - register 2
-   16..19 - register 3
-   20..27 - num8
-   28..29 - bitwidth
-   30..63 - reserved

-   0..63 - num64

<table border="2" cellspacing="0" cellpadding="6" rules="groups" frame="hsides">


<colgroup>
<col  class="org-right" />

<col  class="org-left" />

<col  class="org-left" />

<col  class="org-left" />
</colgroup>
<thead>
<tr>
<th scope="col" class="org-right">№</th>
<th scope="col" class="org-left">Имя</th>
<th scope="col" class="org-left">Аргументы</th>
<th scope="col" class="org-left">Описание</th>
</tr>
</thead>
<tbody>
<tr>
<td class="org-right">0</td>
<td class="org-left">sto</td>
<td class="org-left">r r num64</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">1</td>
<td class="org-left">loa</td>
<td class="org-left">r r num64</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">2</td>
<td class="org-left">add</td>
<td class="org-left">r r r</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">3</td>
<td class="org-left">sub</td>
<td class="org-left">r r r</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">4</td>
<td class="org-left">mul</td>
<td class="org-left">r r r</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">5</td>
<td class="org-left">div</td>
<td class="org-left">r r r</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">6</td>
<td class="org-left">add</td>
<td class="org-left">r r num64</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">7</td>
<td class="org-left">sub</td>
<td class="org-left">r r num64</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">8</td>
<td class="org-left">mul</td>
<td class="org-left">r r num64</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">9</td>
<td class="org-left">div</td>
<td class="org-left">r r num64</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">10</td>
<td class="org-left">addz</td>
<td class="org-left">r r num64</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">11</td>
<td class="org-left">addc</td>
<td class="org-left">r r num64</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">12</td>
<td class="org-left">adds</td>
<td class="org-left">r r num64</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">13</td>
<td class="org-left">not</td>
<td class="org-left">r r</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">14</td>
<td class="org-left">and</td>
<td class="org-left">r r r</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">15</td>
<td class="org-left">or</td>
<td class="org-left">r r r</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">16</td>
<td class="org-left">xor</td>
<td class="org-left">r r r</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">17</td>
<td class="org-left">shl</td>
<td class="org-left">r r r</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">18</td>
<td class="org-left">shr</td>
<td class="org-left">r r r</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">19</td>
<td class="org-left">and</td>
<td class="org-left">r r num64</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">20</td>
<td class="org-left">or</td>
<td class="org-left">r r num64</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">21</td>
<td class="org-left">xor</td>
<td class="org-left">r r num64</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">22</td>
<td class="org-left">shl</td>
<td class="org-left">r r num64</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">23</td>
<td class="org-left">shr</td>
<td class="org-left">r r num64</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">24</td>
<td class="org-left">push</td>
<td class="org-left">r</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">25</td>
<td class="org-left">pop</td>
<td class="org-left">r</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">26</td>
<td class="org-left">call</td>
<td class="org-left">r</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">27</td>
<td class="org-left">int</td>
<td class="org-left">num8</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">28</td>
<td class="org-left">iret</td>
<td class="org-left">&#xa0;</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">29</td>
<td class="org-left">chst</td>
<td class="org-left">r</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">30</td>
<td class="org-left">lost</td>
<td class="org-left">r</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">31</td>
<td class="org-left">chtp</td>
<td class="org-left">r</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">32</td>
<td class="org-left">lotp</td>
<td class="org-left">r</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">33</td>
<td class="org-left">chflag</td>
<td class="org-left">r</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">34</td>
<td class="org-left">loflag</td>
<td class="org-left">r</td>
<td class="org-left">&#xa0;</td>
</tr>

<tr>
<td class="org-right">35</td>
<td class="org-left">utok</td>
<td class="org-left">r r</td>
<td class="org-left">переносит r3 из user r.f. в r1 из kernel r.f.</td>
</tr>

<tr>
<td class="org-right">36</td>
<td class="org-left">ktou</td>
<td class="org-left">r r</td>
<td class="org-left">&#xa0;</td>
</tr>
</tbody>
</table>


<a id="org74aa681"></a>

# Прерывания

выполнение прерывания включает флаг ISINTERRUPT, который переключает ядро в KERNELMODE

из програмных прерываний я вижу смысл только в двух:

-   syscall(отдельная инструкция, адрес обработчика хранится в ядре)
-   переключение контекста(вызывается другим ядром, которое обрабатывает прерывание от таймера)

остальные прерывания вызываются аппаратно, поэтому далее речь будет иммено про них

На каком ядре будет вызвано прерывание решает APIC на процессоре(у него есть таблица, которую можно менять)
Внутри ядра прерывания вызываются поочереди, прерывать прерывание нельзя.

(Интерестный вопрос: если прерываня на ядре выключены, но какое-нибудь устройство его отправило, то прерыване просто игнорировать или куда-нибудь сохранять?)

