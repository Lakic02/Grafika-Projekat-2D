# Grafika-Projekat-2D
7. Bioskop
Napraviti 2D grafičku aplikaciju za upravljanje sedištima u bioskopu.

Na ekranu je prikazana ptičja perspektiva bioskopske sale sa najmanje 50 sedišta. Ulaz u salu se nalazi na gornjem levom delu ekrana, a bioskopsko platno u vidu horizontalnog belog pravougaonika na gornjem delu ekrana. Preko celog ekrana postoji tamnosivi pravougaonik sa providnošću 0,5.

Na početku su sva sedišta obojena u plavo, odnosno imaju status Slobodno. Levim klikom miša na bilo koje od sedišta, sedište postaje žute boje, odnosno ima status Rezervisano. Ukoliko se opet levim klikom miša stisne na neko rezervisano sedište, rezervacija se otkaže i sedište opet ima status Slobodno.

Klikom na tastere od 1 do 9 se mogu kupiti karte. Kada se klikne broj N, potrebno je krenuti od najdešnjeg sedišta u poslednjem redu i, kada se nađe prvih N susednih sedišta koja imaju status Slobodno i ona postaju crvene boje, odnosno dobijaju status Kupljeno.

Klikom na taster Enter se simulira početak projekcije i tada se vrata sale otvaraju i nestaje veliki providni tamnosivi pravougaonik. Sedišta više ne menjaju status, u salu ulaze ljudi, prikazani sličicom čovečuljka, a ima ih najviše onoliko koliko ima ukupno rezervisanih i kupljenih sedišta (npr. moguće je da je rezervisano 5 mesta i kupljeno 7 a da uđe 10 ljudi, broj treba da bude random). Svaka osoba koja uđe se konstantnom brzinom kreće do jednog od mesta, tako što prvo ide vertikalno do svog reda, a zatim horizontalno do svog sedišta.

Kada su svi ljudi ušli i seli na svoja sedišta, vrata se zatvaraju i film počinje. Film traje 20 sekundi, a za to vreme se bioskopsko platno menja na sledeći način: menja se u različitu nasumičnu boju na svakih 20 frejmova. Nakon što prođe 20 sekundi, platno postaje bele boje, vrata se otvore i ljudi kreću da izlaze istom putanjom kojom su došli do sedišta. Kada svi izađu, vrata se zatvore i vrati se tamnosivi pravougaonik, odnosno, program se vraća na stanje s početka programa (ponovo je moguća rezervacija i kupovina).

Kursor treba da ima izgled filmske kamere.
