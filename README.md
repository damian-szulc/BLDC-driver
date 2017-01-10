# Sterowanie bezczujnikowe silnikiem BLDC

## Cel projektu

Celem projektu jest zrealizowanie bezczujnikowego sterowania w pętli otwartej silnikiem bezszczotkowym prądu stałego, bazując na elementach zestawu P-NUCLEO-IHM001. Dodatkowo należy wykonać graficzny interfejs użytkownika do komunikacji i sterowania ze wspomnianym wyżej układem, za pomocą komputerów klasy PC.

W projekcie wykorzystano gotowy zestaw P-NUCLEO-IHM001, w skład, którego wchodzi:
* płytka startowa z serii STM32 NUCLEO wyposażona w mikrokontroler STM32F302R8,
* płytka rozszerzająca X-NUCLEO-IHM07M1 ze sterownikiem trójfazowym silnika bezszczotkowego,
* silnik bezszczotkowy BR2804 1700Kv-1.

## Firmware

W ramach projektu całe oprogramowanie dla mikrokontrolera STM32F302R8 napisano w języku C z wykorzystaniem, dostarczonej przez producenta, biblioteki HAL (ang. Hardware Abstraction Layer). Dodatkowo skorzystano z aplikacji STM32CubeMX, będącej narzędziem graficznym wspomagającym proces konfiguracji mikrokontrolerów STM32 (w tym ich peryferii oraz komponentów middleware). 

## GUI

W celu realizacji interfejsu graficznego wykorzystano środowisko Qt. Całość aplikacji napisano w języku C++. Dodatkowo w celu prezentacji graficznej przebiegów wypełnienia oraz estymowanej wartości prędkości na wykresach, wykorzystano widżet QCustomPlot dostępny w ramach licencji GPL. W celu nawiązania komunikacji z mikrokontrolerem konieczne jest zainstalowania sterowników, dla środowiska Windows oznaczonych, jako STSW-LINK009, dostępny do pobrania na stronie producenta STMicroelectronics. Do obsługi komunikacji szeregowej wykorzystano bibliotekę QSerialPort.
