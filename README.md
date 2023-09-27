# Animals-and-Hunters
Bu projem sayesinde thread leri daha iyi ögrendim.

simülasyonda aşağıdaki üç tür hayvan vardır.
a bird,
a bear,
a panda
bu hayvanlar rastgele gezmekte ve farklı tiplerdeki sitelara haraket etmektedirler.

farklı tipteki sitelarda aşağıdaki şekilde davranmaktadırlar:

NESTİNG siteda kendi cinsinden bir tane daha hayvan oluşturmaktadır ve random komşu lokasyona gitmektedir.

WINTERING siteda tüm hayvanlar 0.5 ihtimalle ölmektedirler. Yadarandom komşu lokasyona gitmektedirler.

FEEDING siteda tüm hayvanlar 0.8 ihtimalle kalmakta yada random komşu lokasyona gitmektedirler.

Hunter simülasyonda rastgele hareket eden yine avcılar bulunmaktadır. Eğer avcılar bir site'a giderse bu
bölgedeki tüm hayvanları öldürmektdirler. Öldürülen hayvan sayısı kendi pointleri olarak toplanmaktadır.

Hunter sayısı programa argüman olarak verilmektedir.
./main 2 ifadesi iki hunter oluşturmaktadır.

main threadde her bir hayvan ve her bir hunter için birer thread oluşturulur ve bunlar
rastgele pozisyonlardan başlatılır. Bu threadler 1 milisaniye uyur ve
sonra rastgele komşu pozisyona belirli kurallar çerçevesinde hareket etmektedirler.
