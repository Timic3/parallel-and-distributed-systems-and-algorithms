# Vaje 01

## 1. Napiši ukaz, ki prikaže podrobne informacije o vsakem vozlišču
```bash
# -N  Informacije o vozliščih (nodes)
# -l  Podrobne informacije (long)
[tm5124@nsc-login ~]$ sinfo -N -l
Mon Oct 19 17:06:42 2020
NODELIST    NODES PARTITION       STATE CPUS    S:C:T MEMORY TMP_DISK WEIGHT AVAIL_FE REASON
nsc-fp001       1 gridlong*    drained* 16      2:8:1  64200        0   1000 intel,gp reinstall
nsc-fp002       1 gridlong*   allocated 16      2:8:1  64200        0   1000 intel,gp none
nsc-fp003       1 gridlong*   allocated 16      2:8:1  64200        0   1000 intel,gp none
nsc-fp004       1 gridlong*   allocated 16      2:8:1  64200        0   1000 intel,gp none
nsc-fp005       1 gridlong*   allocated 16      2:8:1  64200        0   1000 intel,gp none
nsc-fp006       1 gridlong*   allocated 16      2:8:1  64200        0   1000 intel,gp none
nsc-fp007       1 gridlong*   allocated 16      2:8:1  64200        0   1000 intel,gp none
nsc-fp008       1 gridlong*   allocated 16      2:8:1  64200        0   1000 intel,gp none
nsc-gsv001      1 gridlong*   allocated 64     4:16:1 515970        0      1 AMD,bigm none
nsc-gsv002      1 gridlong*   allocated 64     4:16:1 515970        0      1 AMD,bigm none
nsc-gsv003      1 gridlong*   allocated 64     4:16:1 515970        0      1 AMD,bigm none
nsc-gsv004      1 gridlong*   allocated 64     4:16:1 515970        0      1 AMD,bigm none
nsc-gsv005      1 gridlong*   allocated 64     4:16:1 515970        0      1 AMD,bigm none
nsc-gsv006      1 gridlong*   allocated 64     4:16:1 515970        0      1 AMD,bigm none
nsc-gsv007      1 gridlong*   allocated 64     4:16:1 515970        0      1 AMD,bigm none
nsc-msv001      1 gridlong*    drained* 64     4:16:1 257920        0      1      AMD reinstall
nsc-msv002      1 gridlong*    reserved 64     4:16:1 257920        0      1      AMD none
nsc-msv003      1 gridlong*   allocated 64     4:16:1 257920        0      1      AMD none
nsc-msv004      1 gridlong*   allocated 64     4:16:1 257920        0      1      AMD none
nsc-msv005      1 gridlong*   allocated 64     4:16:1 257920        0      1      AMD none
nsc-msv006      1 gridlong*   allocated 64     4:16:1 257920        0      1      AMD none
nsc-msv007      1 gridlong*   allocated 64     4:16:1 257920        0      1      AMD none
nsc-msv008      1 gridlong*       mixed 64     4:16:1 257920        0      1      AMD none
nsc-msv009      1 gridlong*   allocated 64     4:16:1 257920        0      1      AMD none
nsc-msv010      1 gridlong*   allocated 64     4:16:1 257920        0      1      AMD none
nsc-msv011      1 gridlong*   allocated 64     4:16:1 257920        0      1      AMD none
nsc-msv012      1 gridlong*   allocated 64     4:16:1 257920        0      1      AMD none
nsc-msv013      1 gridlong*   allocated 64     4:16:1 257920        0      1      AMD none
nsc-msv014      1 gridlong*   allocated 64     4:16:1 257920        0      1      AMD none
nsc-msv015      1 gridlong*   allocated 64     4:16:1 257920        0      1      AMD none
nsc-msv016      1 gridlong*   allocated 64     4:16:1 257920        0      1      AMD none
nsc-msv017      1 gridlong*   allocated 64     4:16:1 257920        0      1      AMD none
nsc-msv018      1 gridlong*       mixed 64     4:16:1 257920        0      1      AMD none
nsc-msv019      1 gridlong*    reserved 64     4:16:1 257920        0      1      AMD none
nsc-msv020      1 gridlong*   allocated 64     4:16:1 257920        0      1      AMD none
```

`nsc-fp`, `nsc-gsv` in `nsc-msv` so vozlišča - mi bomo uporabljali le 2. Vidimo, da `nsc-fp` ima 16 procesorjev (Intel Xeon), drugi pa 64 (AMD Opteron).

- **AVAIL_FE**: Kaj se na vozlišču nahaja.
  - `intel`/`AMD` - Ima CPU
  - `gp` - Ima GPU
  - `bigm`/`bigmemory` - Več RAMa na strežniku.

- **MEMORY**:
  - `nsc-fp` - Ima 64GB
  - `nsc-gp` - Ima 512GB
  - `nsc-msv` - Ima 256GB (ni `bigm` oznake).

- **STATE**: Stanje vozlišča
  - `idle` - Nič se ne dogaja
  - `mixed` - Nekaj teče, ampak ni izkoriščen - nekaj CPUjev je še zmeraj prostih.
  - `reserved` - Aktivna rezervacija - naša - `nsc-msv019` in `nsc-msv002`.

## 2. Napiši ukaz, ki prikaže izvajalne konfiguracije (features) in vozlišča, ki so na voljo
```bash
# -o      Formatiramo izhod
# '%20N'  Vsak stolpec širok 20 znakov (nodes)
# '%f'    Izpiši features
[tm5124@nsc-login ~]$ sinfo -o "%20N %f"
NODELIST             AVAIL_FEATURES
nsc-fp[001-008]      intel,gpu,k40
nsc-msv[001-020]     AMD
nsc-gsv[001-007]     AMD,bigmem
# K40 - Nvidia Tesla K40
```

## 3. Napiši ukaz, ki prikaže aktivne rezervacije na gruči
```bash
[tm5124@nsc-login ~]$ sinfo -T
RESV_NAME     STATE           START_TIME             END_TIME     DURATION  NODELIST
upgrade2   INACTIVE  2020-10-20T08:30:01  2021-10-20T08:30:01  365-00:00:00  nsc-msv[003,008,013,015,018,020]
fri          ACTIVE  2020-10-13T13:57:32  2021-10-13T13:57:32  365-00:00:00  nsc-msv[002,019]
```

Mi imamo rezerviranega `nsc-msv002` in `nsc-msv019`.

## 4. Izpiši podrobnosti o vozlišču, ki se nahaja v rezervaciji fri
```bash
[tm5124@nsc-login ~]$ scontrol show nodes nsc-msv002
NodeName=nsc-msv002 Arch=x86_64 CoresPerSocket=16
   CPUAlloc=0 CPUTot=64 CPULoad=0.00
   AvailableFeatures=AMD
   ActiveFeatures=AMD
   Gres=(null)
   NodeAddr=nsc-msv002 NodeHostName=nsc-msv002 Version=20.02.5
   OS=Linux 5.7.12-1.el8.elrepo.x86_64 #1 SMP Fri Jul 31 16:22:54 EDT 2020
   RealMemory=257920 AllocMem=0 FreeMem=247975 Sockets=4 Boards=1
   State=RESERVED ThreadsPerCore=1 TmpDisk=0 Weight=1 Owner=N/A MCS_label=N/A
   Partitions=gridlong
   BootTime=2020-10-12T15:02:27 SlurmdStartTime=2020-10-13T11:39:28
   CfgTRES=cpu=64,mem=257920M,billing=64
   AllocTRES=
   CapWatts=n/a
   CurrentWatts=0 AveWatts=0
   ExtSensorsJoules=n/s ExtSensorsWatts=0 ExtSensorsTemp=n/s
```

Na tej matični plošči so fizično 4 procesorji (4 podnožja) in vsak procesor ima 16 jeder. `CPUTot / CoresPerSocket`.

## 5. Izpiši vse posle, ki se trenutno izvajajo na gruči
```bash
[tm5124@nsc-login ~]$ squeue
             JOBID PARTITION     NAME     USER ST       TIME  NODES NODELIST(REASON)
            273236  gridlong mc16_13T prdatlas PD       0:00      1 (Priority)
            273231  gridlong mc16_13T prdatlas PD       0:00      1 (Priority)
            273230  gridlong mc16_13T prdatlas PD       0:00      1 (Priority)
            273226  gridlong mc16_13T prdatlas PD       0:00      1 (Priority)
            273225  gridlong mc16_13T prdatlas PD       0:00      1 (Priority)
            ...
```
- **JOBID**: Vsak posel, ki ga zaženemo ima svoj ID.
- **NAME**: Ime posla, ki ga lahko sami dodelimo.
- **USER**: Kdo je zagnal posel.
- **ST**: Stanje posla.
- **TIME**: Koliko časa se že izvaja.
  - `R` - Running - tečejo
- **NODELIST**: Na katerih vozliščih se izvaja.

## 6. Izpiši vse posle, ki so že zaključili z izvajanjem na gruči
```bash
# CD  Completed (lahko tudi R - Running)
[tm5124@nsc-login ~]$ squeue -t CD
             JOBID PARTITION     NAME     USER ST       TIME  NODES NODELIST(REASON)
            273325  gridlong mc16_13T prdatlas CD    5:34:31      1 nsc-msv006
            273107  gridlong mc16_13T prdatlas CD    3:51:06      1 nsc-msv020
            273371  gridlong mc16_13T prdatlas CD    1:59:09      1 nsc-msv005
```

## 7. Zaženi tri instance programa hostname na gruči, pri tem uporabi rezervacijo fri
```bash
# Zaženemo dve instanci na FRI rezervaciji
[tm5124@nsc-login ~]$ srun -n3 --reservation=fri hostname
nsc-msv002.ijs.si
nsc-msv002.ijs.si
nsc-msv019.ijs.si
```

## 8. Zaženi tri instance programa hostname na treh vozliščih gruče
```bash
# Zaženemo tri instance na treh vozliščih (porazdeljeno)
[tm5124@nsc-login ~]$ srun -n3 -N3 hostname
nsc-msv008.ijs.si
nsc-msv020.ijs.si
nsc-msv018.ijs.si
```

## Ekstra
```bash
[tm5124@nsc-login ~]$ hostname
nsc-login.ijs.si
# Zaženemo eno instanco
[tm5124@nsc-login ~]$ srun -n1 hostname
nsc-msv008.ijs.si
# Zaženemo eno instanco na FRI rezervaciji
[tm5124@nsc-login ~]$ srun -n1 --reservation=fri hostname
nsc-msv002.ijs.si
# Zaženemo dve instanci na FRI rezervaciji
[tm5124@nsc-login ~]$ srun -n2 --reservation=fri hostname
nsc-msv002.ijs.si
nsc-msv002.ijs.si
# Zaženemo deset instanc na FRI rezervaciji
[tm5124@nsc-login ~]$ srun -n10 --reservation=fri hostname
nsc-msv002.ijs.si
nsc-msv002.ijs.si
nsc-msv002.ijs.si
nsc-msv002.ijs.si
nsc-msv002.ijs.si
nsc-msv002.ijs.si
nsc-msv002.ijs.si
nsc-msv002.ijs.si
nsc-msv002.ijs.si
nsc-msv002.ijs.si
# Zaženemo štiri instance na dveh vozliščih (porazdeljeno) na FRI rezervaciji
[tm5124@nsc-login ~]$ srun -n4 -N2 --reservation=fri hostname
nsc-msv002.ijs.si
nsc-msv002.ijs.si
nsc-msv019.ijs.si
nsc-msv019.ijs.si
# Zaženemo štiri instance na treh vozliščih (porazdeljeno) na FRI rezervaciji
# Nimamo v rezervaciji treh vozlišč
[tm5124@nsc-login ~]$ srun -n4 -N3 --reservation=fri hostname
srun: error: Unable to allocate resources: Requested node configuration is not available
# Izven rezervacije nam pa dovoli
[tm5124@nsc-login ~]$ srun -n4 -N3 hostname
nsc-msv008.ijs.si
nsc-msv008.ijs.si
nsc-msv020.ijs.si
nsc-msv018.ijs.si
```

## 9. Prikaži vse tvoje zaključene posle
```bash
[tm5124@nsc-login ~]$ squeue -u tm5124 -t CD
             JOBID PARTITION     NAME     USER ST       TIME  NODES NODELIST(REASON)
            273427  gridlong hostname   tm5124 CD       0:00      1 nsc-msv002
            273428  gridlong hostname   tm5124 CD       0:00      2 nsc-msv[002,019]
            273425  gridlong hostname   tm5124 CD       0:01      1 nsc-msv002
            273424  gridlong hostname   tm5124 CD       0:01      1 nsc-msv002
            273423  gridlong hostname   tm5124 CD       0:01      1 nsc-msv008
            273431  gridlong hostname   tm5124 CD       0:00      3 nsc-msv[008,018,020]
```

## 10. Zahtevaj vozlišče, ki vsebuje enote GPU in na njem zaženi program `nvidia-smi`
```bash
# -G  Koliko GPUjev
# -C  Tipi vozlišč
[tm5124@nsc-login ~]$ srun -n1 -G1 -C gpu nvidia-smi
# Informacije o grafičnih karticah
```

## 11. Zaženi interaktivno lupino na enem izmed vozlišč in v njej poženi `env | grep SLURM`
**Kakšen je identifikator posla trenutne seje?** `SLURM_JOB_ID=273438`
```bash
# Zaženemo lupino na vozlišču
[tm5124@nsc-login ~]$ srun -n1 --reservation=fri --pty bash -i

[tm5124@nsc-msv002 ~]$ ls
Vaje01.md
[tm5124@nsc-msv002 ~]$ hostname
nsc-msv002.ijs.si
[tm5124@nsc-msv002 ~]$ exit
exit

[tm5124@nsc-login ~]$
```

Lahko uporabljamo (v splošni vrsti) karkoli računskega, **Bitcoinov ne smemo rudarit!**

## 12. Napiši skripto za opis posla in jo zaženi z ukazom `sbatch`
`first_job.sh`
```bash
#!/bin/bash

#SBATCH --job-name=my_first_time // Ime posla
#SBATCH --ntasks=4 // Koliko nalog
#SBATCH --nodes=1 // Koliko vozlišč
#SBATCH --reservation=fri // Rezervacija
#SBATCH --mem-per-cpu=100MB // 100MB na CPU
#SBATCH --output=job.out // Kam se bo izpisalo
#SBATCH --time=00:01:00 // Maksimalen čas posla

srun hostname
```

```bash
[tm5124@nsc-login vaja01]$ sbatch first_job.sh
Submitted batch job 273440
[tm5124@nsc-login vaja01]$ squeue -u tm5124 -t CD
             JOBID PARTITION     NAME     USER ST       TIME  NODES NODELIST(REASON)
            273437  gridlong     bash   tm5124 CD       2:03      1 nsc-msv002
            273438  gridlong     bash   tm5124 CD       0:47      1 nsc-msv002
            273440  gridlong first-jo   tm5124 CD       0:01      1 nsc-msv002
[tm5124@nsc-login vaja01]$ cat job.out
nsc-msv002.ijs.si
nsc-msv002.ijs.si
nsc-msv002.ijs.si
nsc-msv002.ijs.si
```

`job.out`
```
nsc-msv002.ijs.si
nsc-msv002.ijs.si
nsc-msv002.ijs.si
nsc-msv002.ijs.si
```