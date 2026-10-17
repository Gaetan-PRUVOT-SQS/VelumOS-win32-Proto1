# Matrice de couverture fonctionnelle

Fichier généré par `tools/matrice_couverture.py` à partir de [`matrice.toml`](matrice.toml). Ne pas l'éditer à la main.
La commande vérifie aussi que chaque test cité existe.

## Définitions

La matrice recense les fonctionnalités vérifiables de la première tranche de VelumOS, regroupées par sous-système
(voir le README pour la table a01 à a20). Chaque fonctionnalité a un mode de vérification :

| Mode | Sens |
|------|------|
| A | vérifiée par des tests automatisés, avec un oracle automatique (valeur attendue, modèle de référence, journal série, capture comparée) |
| AM | la logique est vérifiée automatiquement, un complément manuel couvre ce qui demande un jugement humain (aspect visuel, ergonomie, vrai clavier) |
| M | vérifiée seulement à la main : aucun test automatisé dans le dépôt |

La couverture automatisée est la part des fonctionnalités en mode A ou AM. La part manuelle seule est celle en mode M.
Les cas manuels sont décrits dans [`MANUEL.md`](MANUEL.md).

## Résultat

| Mesure | Valeur |
|--------|--------|
| Fonctionnalités vérifiables recensées | 103 |
| Couverture automatisée (A + AM) | **93,2 %** (96 sur 103) |
| dont automatisée seule (A) | 82,5 % (85) |
| dont automatisée avec complément manuel (AM) | 10,7 % (11) |
| Couverture manuelle seule (M) | **6,8 %** (7 sur 103) |
| Fonctionnalités qui demandent au moins un test manuel (AM + M) | 17,5 % (18) |
| Cas de test automatisés (cas unitaires, scénarios QEMU, tests Python) | 2196 |
| Cas de test manuels | 22 |
| Cas manuels joués | 19 sur 22 (15 réussi, 4 partiel) |
| Taux d'automatisation par nombre de cas | 99,0 % |

Lecture : le taux par nombre de cas est très supérieur au taux fonctionnel parce que les cas automatisés sont fins
(une valeur limite, une partition) alors que les cas manuels sont des sessions complètes. Le taux fonctionnel est
la mesure à retenir.

Inventaire des cas automatisés : 732 fichiers de tests unitaires contenant 2046 cas nommés
(`h_run`), 50 scénarios QEMU et 100 tests Python.

## Par sous-système

| Sous-système | Fonctionnalités | A | AM | M | Automatisée (A + AM) |
|---|---|---|---|---|---|
| a01 | 4 | 4 | 0 | 0 | 100,0 % |
| a02 | 4 | 4 | 0 | 0 | 100,0 % |
| a03 | 4 | 4 | 0 | 0 | 100,0 % |
| a04 | 3 | 3 | 0 | 0 | 100,0 % |
| a05 | 5 | 4 | 0 | 1 | 80,0 % |
| a06 | 4 | 4 | 0 | 0 | 100,0 % |
| a07 | 5 | 4 | 1 | 0 | 100,0 % |
| a08 | 4 | 4 | 0 | 0 | 100,0 % |
| a09 | 3 | 3 | 0 | 0 | 100,0 % |
| a10 | 3 | 2 | 1 | 0 | 100,0 % |
| a11 | 2 | 2 | 0 | 0 | 100,0 % |
| a12 | 2 | 1 | 1 | 0 | 100,0 % |
| a13 | 5 | 3 | 2 | 0 | 100,0 % |
| a14 | 3 | 3 | 0 | 0 | 100,0 % |
| a15 | 4 | 3 | 0 | 1 | 75,0 % |
| a16 | 4 | 3 | 0 | 1 | 75,0 % |
| a17 | 8 | 7 | 0 | 1 | 87,5 % |
| a18 | 5 | 4 | 1 | 0 | 100,0 % |
| a19 | 5 | 4 | 1 | 0 | 100,0 % |
| a20 | 7 | 4 | 3 | 0 | 100,0 % |
| transverse | 7 | 3 | 1 | 3 | 57,1 % |
| apk | 12 | 12 | 0 | 0 | 100,0 % |

## Matrice complète

| ID | Sous-système | Fonctionnalité | Mode | Tests automatisés | Tests manuels |
|---|---|---|---|---|---|
| F01 | a01 | Encodage des descripteurs GDT, TSS et IDT | A | [test_gdt.c](host/a01/test_gdt.c)<br>[test_idt.c](host/a01/test_idt.c)<br>[test_layout.c](host/a01/test_layout.c) | aucun |
| F02 | a01 | Décodage CPUID et activation des protections (NX, SMEP, SMAP, UMIP) | A | [test_cpuid.c](host/a01/test_cpuid.c)<br>[test_cpuid_bits.c](host/a01/test_cpuid_bits.c)<br>[a01_boot.py](qemu/a01_boot.py)<br>[a01_boot_qemu64.py](qemu/a01_boot_qemu64.py) | aucun |
| F03 | a01 | Exceptions noyau avec registres exacts (#DE, #GP, #UD, #PF, #NMI) | A | [test_trap.c](host/a01/test_trap.c)<br>[a01_fault_div0.py](qemu/a01_fault_div0.py)<br>[a01_fault_gp.py](qemu/a01_fault_gp.py)<br>[a01_fault_ud.py](qemu/a01_fault_ud.py)<br>[a01_fault_pf.py](qemu/a01_fault_pf.py)<br>[a01_fault_nmi.py](qemu/a01_fault_nmi.py) | aucun |
| F04 | a01 | Double faute sur pile dédiée (récursion infinie) et trace d'appels | A | [test_backtrace.c](host/a01/test_backtrace.c)<br>[a01_fault_df.py](qemu/a01_fault_df.py)<br>[a01_fault_stack.py](qemu/a01_fault_stack.py) | aucun |
| F05 | a02 | Allocation et libération de frames (alignement, plages, limites) | A | [test_alloc_pages.c](host/a02/test_alloc_pages.c)<br>[test_alloc_zone.c](host/a02/test_alloc_zone.c)<br>[test_free.c](host/a02/test_free.c)<br>[test_nextfit.c](host/a02/test_nextfit.c)<br>[test_unit_range.c](host/a02/test_unit_range.c) | aucun |
| F06 | a02 | Cohérence face à une suite aléatoire (modèle de référence, métamorphique) | A | [test_model.c](host/a02/test_model.c)<br>[test_metamorphic.c](host/a02/test_metamorphic.c)<br>[test_states.c](host/a02/test_states.c) | aucun |
| F07 | a02 | Détection des libérations invalides (double, mauvais propriétaire, adresse) | A | [test_free_faults.c](host/a02/test_free_faults.c)<br>[a02_fault_double_free.py](qemu/a02_fault_double_free.py)<br>[a02_fault_wrong_owner.py](qemu/a02_fault_wrong_owner.py)<br>[a02_fault_bad_align.py](qemu/a02_fault_bad_align.py)<br>[a02_fault_bad_range.py](qemu/a02_fault_bad_range.py) | aucun |
| F08 | a02 | Budget des métadonnées et gestion de plus de 4 Gio | A | [test_budget.c](host/a02/test_budget.c)<br>[a02_mem4g.py](qemu/a02_mem4g.py) | aucun |
| F09 | a03 | Mappage, protection et W^X | A | [test_map.c](host/a03/test_map.c)<br>[test_protect.c](host/a03/test_protect.c)<br>[test_unmap.c](host/a03/test_unmap.c)<br>[test_audit.c](host/a03/test_audit.c) | aucun |
| F10 | a03 | Copies utilisateur sans faute possible | A | [test_user.c](host/a03/test_user.c)<br>[test_ustr.c](host/a03/test_ustr.c)<br>[test_check.c](host/a03/test_check.c) | aucun |
| F11 | a03 | Échec d'allocation en cours d'opération sans fuite | A | [test_fail.c](host/a03/test_fail.c)<br>[test_pool.c](host/a03/test_pool.c)<br>[test_model.c](host/a03/test_model.c) | aucun |
| F12 | a03 | Page de garde et faute de page noyau | A | [a03_guard.py](qemu/a03_guard.py)<br>[a03_vmm.py](qemu/a03_vmm.py)<br>[test_ustack.c](host/a03/test_ustack.c)<br>[test_reserve.c](host/a03/test_reserve.c) | aucun |
| F13 | a04 | Allocation par classes, alignements, redimensionnement | A | [test_class_small.c](host/a04/test_class_small.c)<br>[test_class_large.c](host/a04/test_class_large.c)<br>[test_align.c](host/a04/test_align.c)<br>[test_realloc.c](host/a04/test_realloc.c) | aucun |
| F14 | a04 | Détection de corruption et de double libération | A | [test_check_debug.c](host/a04/test_check_debug.c)<br>[test_free_debug.c](host/a04/test_free_debug.c)<br>[test_check_slab.c](host/a04/test_check_slab.c) | aucun |
| F15 | a04 | Absence de fuite et injection d'échec à chaque rang | A | [test_fail_rank.c](host/a04/test_fail_rank.c)<br>[test_stats.c](host/a04/test_stats.c)<br>[test_meta.c](host/a04/test_meta.c)<br>[a04_heap.py](qemu/a04_heap.py)<br>[a04_heap_petite_ram.py](qemu/a04_heap_petite_ram.py) | aucun |
| F16 | a05 | Analyse des tables ACPI (RSDP, MADT, FADT, _S5), y compris corrompues | A | [test_acpi_rsdp.c](host/a05/test_acpi_rsdp.c)<br>[test_acpi_madt.c](host/a05/test_acpi_madt.c)<br>[test_acpi_fadt.c](host/a05/test_acpi_fadt.c)<br>[test_acpi_s5.c](host/a05/test_acpi_s5.c)<br>[test_acpi_sdt.c](host/a05/test_acpi_sdt.c)<br>[test_acpi_parse.c](host/a05/test_acpi_parse.c) | aucun |
| F17 | a05 | Routage IOAPIC et gestion des IRQ | A | [test_ioapic_enc.c](host/a05/test_ioapic_enc.c)<br>[test_irq_route.c](host/a05/test_irq_route.c)<br>[test_irq_core.c](host/a05/test_irq_core.c) | aucun |
| F18 | a05 | Conversion TSC en nanosecondes, RTC, minuteries | A | [test_tsc_conv.c](host/a05/test_tsc_conv.c)<br>[test_rtc.c](host/a05/test_rtc.c)<br>[test_civil.c](host/a05/test_civil.c)<br>[test_theap.c](host/a05/test_theap.c)<br>[test_wait.c](host/a05/test_wait.c) | aucun |
| F19 | a05 | Extinction ACPI | A | [a20_souris.py](qemu/a20_souris.py) | aucun |
| F20 | a05 | Redémarrage ACPI (Automatisable avec l'option power=reboot, scénario non écrit.) | M | aucun | MT-09 |
| F21 | a06 | Politique de priorités, files et vieillissement | A | [test_policy_prio.c](host/a06/test_policy_prio.c)<br>[test_policy_pick.c](host/a06/test_policy_pick.c)<br>[test_runq.c](host/a06/test_runq.c) | aucun |
| F22 | a06 | Verrous, mutex avec héritage de priorité, événements, sémaphores | A | [test_spin_excl.c](host/a06/test_spin_excl.c)<br>[test_spin_fifo.c](host/a06/test_spin_fifo.c)<br>[test_mutex.c](host/a06/test_mutex.c)<br>[test_mutex_pi.c](host/a06/test_mutex_pi.c)<br>[test_event.c](host/a06/test_event.c)<br>[test_sem.c](host/a06/test_sem.c) | aucun |
| F23 | a06 | Attentes avec délai et annulation, équité et préemption en noyau | A | [test_waitq_fifo.c](host/a06/test_waitq_fifo.c)<br>[test_waitq_tmo.c](host/a06/test_waitq_tmo.c)<br>[test_waitq_cancel.c](host/a06/test_waitq_cancel.c)<br>[test_pingpong.c](host/a06/test_pingpong.c)<br>[a06_sched.py](qemu/a06_sched.py) | aucun |
| F24 | a06 | Détection d'interblocage | A | [test_lockdep.c](host/a06/test_lockdep.c)<br>[test_spin_stuck.c](host/a06/test_spin_stuck.c) | aucun |
| F25 | a07 | Chargeur ELF face à des fichiers hostiles | A | [test_elf_hdr.c](host/a07/test_elf_hdr.c)<br>[test_elf_seg.c](host/a07/test_elf_seg.c)<br>[test_elf_dyn.c](host/a07/test_elf_dyn.c)<br>[test_elf_fuzz.c](host/a07/test_elf_fuzz.c)<br>[test_elf_real.c](host/a07/test_elf_real.c) | aucun |
| F26 | a07 | Validation des arguments des appels système | A | [test_syscall_table.c](host/a07/test_syscall_table.c)<br>[test_sysret.c](host/a07/test_sysret.c)<br>[test_args.c](host/a07/test_args.c)<br>[a07_ring3test.py](qemu/a07_ring3test.py) | aucun |
| F27 | a07 | Fautes utilisateur sans conséquence pour le noyau | A | [a07_ring3test.py](qemu/a07_ring3test.py)<br>[a07_selftest.py](qemu/a07_selftest.py)<br>[a07_init_absent.py](qemu/a07_init_absent.py) | aucun |
| F28 | a07 | Préemption et arrêt d'un processus qui boucle en anneau 3 | A | [int_preempt.py](qemu/int_preempt.py) | aucun |
| F29 | a07 | Supervision et relance des services de session par init (La mort réelle de winsrv est provoquée par l'option de test init.mort-winsrv et jouée par int_winsrv_mort.) | AM | [test_init_policy.c](host/a07/test_init_policy.c)<br>[int_session.py](qemu/int_session.py)<br>[test_init_killtest.c](host/a07/test_init_killtest.c)<br>[test_init_session.c](host/a07/test_init_session.c)<br>[int_winsrv_mort.py](qemu/int_winsrv_mort.py) | MT-20 |
| F30 | a08 | Table de handles, droits et génération | A | [test_htab.c](host/a08/test_htab.c)<br>[test_htab2.c](host/a08/test_htab2.c)<br>[test_object.c](host/a08/test_object.c) | aucun |
| F31 | a08 | Canaux, messages et transfert de handles | A | [test_chan.c](host/a08/test_chan.c)<br>[test_msgq.c](host/a08/test_msgq.c)<br>[test_xfer.c](host/a08/test_xfer.c)<br>[test_port.c](host/a08/test_port.c)<br>[test_sysipc.c](host/a08/test_sysipc.c) | aucun |
| F32 | a08 | Attente multiple sans réveil perdu | A | [test_wait.c](host/a08/test_wait.c)<br>[test_waitsim.c](host/a08/test_waitsim.c)<br>[test_pulse.c](host/a08/test_pulse.c) | aucun |
| F33 | a08 | Sections de mémoire partagée | A | [test_section.c](host/a08/test_section.c)<br>[test_section2.c](host/a08/test_section2.c)<br>[test_ipcmem.c](host/a08/test_ipcmem.c)<br>[a08_objets.py](qemu/a08_objets.py)<br>[test_section3.c](host/a08/test_section3.c)<br>[test_section4.c](host/a08/test_section4.c)<br>[test_section5.c](host/a08/test_section5.c)<br>[test_section6.c](host/a08/test_section6.c)<br>[int_endurance_fenetres.py](qemu/int_endurance_fenetres.py) | aucun |
| F34 | a09 | Énumération PCI, BAR, capacités, MSI | A | [test_pci_enum_basic.c](host/a09/test_pci_enum_basic.c)<br>[test_pci_enum_bridges.c](host/a09/test_pci_enum_bridges.c)<br>[test_pci_bar.c](host/a09/test_pci_bar.c)<br>[test_pci_cap.c](host/a09/test_pci_cap.c)<br>[test_pci_msi_encode.c](host/a09/test_pci_msi_encode.c)<br>[a09_pci.py](qemu/a09_pci.py)<br>[a09_pci_legacy.py](qemu/a09_pci_legacy.py)<br>[a09_pci_edu.py](qemu/a09_pci_edu.py)<br>[test_pci_intx.c](host/a09/test_pci_intx.c)<br>[test_pci_intx_wr.c](host/a09/test_pci_intx_wr.c) | aucun |
| F35 | a09 | Générateur aléatoire (amorçage, réensemencement, uniformité) | A | [test_rng_core.c](host/a09/test_rng_core.c)<br>[test_rng_reseed.c](host/a09/test_rng_reseed.c)<br>[test_rng_below.c](host/a09/test_rng_below.c)<br>[a09_random.py](qemu/a09_random.py) | aucun |
| F36 | a09 | SHA-256, HMAC et PBKDF2 contre les vecteurs officiels | A | [test_sha256_nist.c](host/a09/test_sha256_nist.c)<br>[test_hmac_rfc4231.c](host/a09/test_hmac_rfc4231.c)<br>[test_pbkdf2_kat.c](host/a09/test_pbkdf2_kat.c)<br>[test_chacha20.c](host/a09/test_chacha20.c) | aucun |
| F37 | a10 | Décodeur clavier (jeu 2) et souris PS/2 avec resynchronisation | A | [test_scan2_basic.c](host/a10/test_scan2_basic.c)<br>[test_scan2_e1.c](host/a10/test_scan2_e1.c)<br>[test_scan2_fuzz.c](host/a10/test_scan2_fuzz.c)<br>[test_mouse_dec.c](host/a10/test_mouse_dec.c)<br>[test_kbd_lost.c](host/a10/test_kbd_lost.c)<br>[test_sys_priv.c](host/a10/test_sys_priv.c) | aucun |
| F38 | a10 | Dispositions AZERTY et QWERTY, touches mortes (Les touches mortes ~ et ` ont été écrites de mémoire, à confirmer sur un vrai clavier.) | AM | [test_layout_fr.c](host/a10/test_layout_fr.c)<br>[test_layout_us.c](host/a10/test_layout_us.c)<br>[test_dead.c](host/a10/test_dead.c) | MT-11 |
| F39 | a10 | Entrées clavier et souris injectées dans QEMU | A | [a10_input.py](qemu/a10_input.py) | aucun |
| F40 | a11 | Partitions GPT et MBR, bornes des E/S | A | [test_gpt.c](host/a11/test_gpt.c)<br>[test_gpt_hdr.c](host/a11/test_gpt_hdr.c)<br>[test_mbr.c](host/a11/test_mbr.c)<br>[test_blk_bounds.c](host/a11/test_blk_bounds.c)<br>[test_ebr.c](host/a11/test_ebr.c)<br>[test_ebr_hostile.c](host/a11/test_ebr_hostile.c)<br>[test_ebr_scan.c](host/a11/test_ebr_scan.c)<br>[a11_etendue.py](qemu/a11_etendue.py) | aucun |
| F41 | a11 | Pilote virtio-blk (lecture, écriture, appareil hostile) | A | [test_virtq.c](host/a11/test_virtq.c)<br>[test_vblk_io.c](host/a11/test_vblk_io.c)<br>[test_vblk_neg.c](host/a11/test_vblk_neg.c)<br>[test_vblk_fault.c](host/a11/test_vblk_fault.c)<br>[a11_virtio_blk.py](qemu/a11_virtio_blk.py)<br>[a11_absent.py](qemu/a11_absent.py) | aucun |
| F42 | a12 | Analyse de l'initrd cpio hostile et des chemins | A | [test_cpio.c](host/a12/test_cpio.c)<br>[test_cpio_big.c](host/a12/test_cpio_big.c)<br>[test_path.c](host/a12/test_path.c) | aucun |
| F43 | a12 | FAT32 en lecture et écriture, noms longs, appels système de fichiers (Scénario QEMU sur une image mtools, relue par mtools et contrôlée par fsck.vfat (code 0 exigé). Tests hôte sur un faux disque en mémoire.) | AM | [a12_fat32.py](qemu/a12_fat32.py)<br>[test_a12_fat32.py](tools/test_a12_fat32.py)<br>[test_fat_fsinfo.c](host/a12/test_fat_fsinfo.c)<br>[test_fat_rw.c](host/a12/test_fat_rw.c)<br>[test_fat_lfn.c](host/a12/test_fat_lfn.c)<br>[test_fat_corrupt.c](host/a12/test_fat_corrupt.c) | MT-23 |
| F44 | a13 | Console noyau : texte, défilement, UTF-8, entrées hostiles | A | [test_kcon_scroll.c](host/a13/test_kcon_scroll.c)<br>[test_kcon_utf8.c](host/a13/test_kcon_utf8.c)<br>[test_kcon_hostile.c](host/a13/test_kcon_hostile.c)<br>[test_kcon_wrap.c](host/a13/test_kcon_wrap.c)<br>[a13_console.py](qemu/a13_console.py) | aucun |
| F45 | a13 | Écran d'arrêt | AM | [test_bsod_paint.c](host/a13/test_bsod_paint.c)<br>[test_bsod_table.c](host/a13/test_bsod_table.c)<br>[a13_panic.py](qemu/a13_panic.py) | MT-21 |
| F46 | a13 | Changement de mode graphique et validation de géométrie | A | [test_dispi_mode.c](host/a13/test_dispi_mode.c)<br>[test_fbgeom.c](host/a13/test_fbgeom.c)<br>[test_display_modes.c](host/a13/test_display_modes.c)<br>[a13_modes.py](qemu/a13_modes.py) | aucun |
| F47 | a13 | Écran de démarrage (barre de progression) | AM | [a13_boot.py](qemu/a13_boot.py)<br>[int_demarrage.py](qemu/int_demarrage.py) | MT-01 |
| F48 | a13 | Appels système d'affichage face à des arguments hostiles | A | [test_sys_fuzz.c](host/a13/test_sys_fuzz.c)<br>[test_sys_map.c](host/a13/test_sys_map.c)<br>[test_sys_setmode.c](host/a13/test_sys_setmode.c) | aucun |
| F49 | a14 | libc minimale (chaînes, nombres, tri, snprintf) | A | [test_strto_bounds.c](host/a14/test_strto_bounds.c)<br>[test_qsort_random.c](host/a14/test_qsort_random.c)<br>[test_snprintf.c](host/a14/test_snprintf.c)<br>[test_ctype.c](host/a14/test_ctype.c) | aucun |
| F50 | a14 | Allocateur utilisateur (corruption, fragmentation, échec) | A | [test_malloc_corrupt.c](host/a14/test_malloc_corrupt.c)<br>[test_malloc_frag.c](host/a14/test_malloc_frag.c)<br>[test_malloc_fail.c](host/a14/test_malloc_fail.c)<br>[test_malloc_churn.c](host/a14/test_malloc_churn.c) | aucun |
| F51 | a14 | Démarrage d'une appli, fils et verrous utilisateur | A | [test_start_parse_fuzz.c](host/a14/test_start_parse_fuzz.c)<br>[test_thread_basic.c](host/a14/test_thread_basic.c)<br>[test_mutex_stress.c](host/a14/test_mutex_stress.c)<br>[a14_vtest.py](qemu/a14_vtest.py) | aucun |
| F52 | a15 | Primitives de dessin et mélange alpha exact | A | [test_fill.c](host/a15/test_fill.c)<br>[test_blit.c](host/a15/test_blit.c)<br>[test_color.c](host/a15/test_color.c)<br>[test_gradient.c](host/a15/test_gradient.c)<br>[test_line.c](host/a15/test_line.c)<br>[test_rrect.c](host/a15/test_rrect.c) | aucun |
| F53 | a15 | Régions de rectangles contre un modèle bitmap | A | [test_region.c](host/a15/test_region.c)<br>[test_region_model.c](host/a15/test_region_model.c) | aucun |
| F54 | a15 | Robustesse du dessin (fuzz, arguments extrêmes) | A | [test_fuzz.c](host/a15/test_fuzz.c)<br>[test_invalid.c](host/a15/test_invalid.c) | aucun |
| F55 | a15 | Performance du dessin plein écran (Bancs écrits, jamais lancés.) | M | aucun | MT-19 |
| F56 | a16 | Décodage UTF-8 strict et recherche de glyphe | A | [test_utf8_valid.c](host/a16/test_utf8_valid.c)<br>[test_utf8_invalid.c](host/a16/test_utf8_invalid.c)<br>[test_utf8_random.c](host/a16/test_utf8_random.c)<br>[test_glyph_lookup.c](host/a16/test_glyph_lookup.c)<br>[test_nbsp.c](host/a16/test_nbsp.c) | aucun |
| F57 | a16 | Mesure et rendu du texte | A | [test_width.c](host/a16/test_width.c)<br>[test_fit.c](host/a16/test_fit.c)<br>[test_render.c](host/a16/test_render.c)<br>[test_draw_clip.c](host/a16/test_draw_clip.c)<br>[a16_console_font.py](qemu/a16_console_font.py)<br>[test_font_bounds.c](host/a16/test_font_bounds.c) | aucun |
| F58 | a16 | Netteté et lisibilité des polices | M | aucun | MT-05, MT-14 |
| F59 | a17 | Partition des zones d'une fenêtre (hit-test) | A | [test_hit_table.c](host/a17/test_hit_table.c)<br>[test_hit_sweep.c](host/a17/test_hit_sweep.c)<br>[test_hit_meta.c](host/a17/test_hit_meta.c) | aucun |
| F60 | a17 | Métriques et géométrie du thème | A | [test_metrics.c](host/a17/test_metrics.c)<br>[test_geo_table.c](host/a17/test_geo_table.c)<br>[test_geo_inv.c](host/a17/test_geo_inv.c) | aucun |
| F61 | a17 | Fidélité visuelle : ancien style Luna (MT-04) et maquette verre (MT-25) (Les tests a17 et a20 vérifient des valeurs calculées depuis la maquette, pas l'aspect d'ensemble.) | M | aucun | MT-04, MT-25 |
| F62 | a18 | Protocole de fenêtres : validation et rejet des messages hostiles | A | [test_core_proto.c](host/a18/test_core_proto.c)<br>[test_srv_fuzz.c](host/a18/test_srv_fuzz.c)<br>[test_srv_faults.c](host/a18/test_srv_faults.c) | aucun |
| F63 | a18 | Ordre Z, focus et activation | A | [test_core_z.c](host/a18/test_core_z.c)<br>[test_srv_focus.c](host/a18/test_srv_focus.c)<br>[test_srv_random.c](host/a18/test_srv_random.c) | aucun |
| F64 | a18 | Composition incrémentale et zones sales | A | [test_core_geom.c](host/a18/test_core_geom.c)<br>[test_srv_random.c](host/a18/test_srv_random.c)<br>[test_srv_queue.c](host/a18/test_srv_queue.c) | aucun |
| F65 | a18 | Glisser, redimensionner, réduire et agrandir une fenêtre à la souris | AM | [test_core_drag.c](host/a18/test_core_drag.c)<br>[test_core_hit.c](host/a18/test_core_hit.c) | MT-06 |
| F66 | a19 | Édition de texte UTF-8 (insertion, sélection, limites) | A | [test_edit.c](host/a19/test_edit.c)<br>[test_edit_sel.c](host/a19/test_edit_sel.c)<br>[test_edit_fuzz.c](host/a19/test_edit_fuzz.c)<br>[test_edit_limit.c](host/a19/test_edit_limit.c) | aucun |
| F67 | a19 | Focus, navigation au clavier et mnémoniques | A | [test_focus.c](host/a19/test_focus.c)<br>[test_focus2.c](host/a19/test_focus2.c)<br>[test_a11y.c](host/a19/test_a11y.c)<br>[test_mnemonic.c](host/a19/test_mnemonic.c)<br>[test_button_kbd.c](host/a19/test_button_kbd.c) | aucun |
| F68 | a19 | Listes et menus | A | [test_list.c](host/a19/test_list.c)<br>[test_list_keys.c](host/a19/test_list_keys.c)<br>[test_menu_keys.c](host/a19/test_menu_keys.c)<br>[test_menu_sub.c](host/a19/test_menu_sub.c) | aucun |
| F69 | a19 | Échec d'allocation et absence de fuite des contrôles | A | [test_alloc_fail.c](host/a19/test_alloc_fail.c)<br>[test_remove.c](host/a19/test_remove.c)<br>[test_storm.c](host/a19/test_storm.c) | aucun |
| F70 | a19 | Accessibilité de la session entière au clavier seul | AM | [test_a11y.c](host/a19/test_a11y.c)<br>[a20_desktop.py](qemu/a20_desktop.py) | MT-13 |
| F71 | a20 | Comptes, hachage PBKDF2 et limiteur d'essais | A | [test_acc_verify.c](host/a20/test_acc_verify.c)<br>[test_acc_hostile.c](host/a20/test_acc_hostile.c)<br>[test_auth.c](host/a20/test_auth.c)<br>[test_limiter.c](host/a20/test_limiter.c) | aucun |
| F72 | a20 | Connexion à la souris, bureau, menu Démarrer, extinction | A | [a20_souris.py](qemu/a20_souris.py)<br>[a20_desktop.py](qemu/a20_desktop.py)<br>[int_session.py](qemu/int_session.py)<br>[int_captures.py](qemu/int_captures.py)<br>[test_programmes.c](host/a20/test_programmes.c) | aucun |
| F73 | a20 | Dock en verre, tâches et liste des fenêtres | AM | [test_taskbar.c](host/a20/test_taskbar.c)<br>[test_taskbar_sweep.c](host/a20/test_taskbar_sweep.c)<br>[test_tasklist.c](host/a20/test_tasklist.c)<br>[test_tasklist_state.c](host/a20/test_tasklist_state.c)<br>[test_contrast.c](host/a20/test_contrast.c) | MT-16 |
| F74 | a20 | Boîtes de dialogue (Exécuter, Éteindre l'ordinateur) | AM | [test_runcmd.c](host/a20/test_runcmd.c)<br>[test_logon_flow.c](host/a20/test_logon_flow.c)<br>[test_sm_model.c](host/a20/test_sm_model.c)<br>[test_typographie.py](host/a20/test_typographie.py) | MT-07, MT-08 |
| F75 | a20 | Horloge de la zone de notification et formats de date | AM | [test_timefmt_clock.c](host/a20/test_timefmt_clock.c)<br>[test_timefmt_uptime.c](host/a20/test_timefmt_uptime.c) | MT-10 |
| F76 | transverse | Construction du noyau, des applis et de l'image bootable (BIOS et UEFI) | A | [skel_boot.py](qemu/skel_boot.py)<br>[test_mkimage.py](tools/test_mkimage.py)<br>[Makefile](../Makefile) | aucun |
| F77 | transverse | Qualité du code : en-têtes autonomes, sécurité statique | A | [scan_securite.py](../tools/scan_securite.py)<br>[test.mk](../mk/test.mk) | aucun |
| F78 | transverse | Variantes de processeur (qemu64 sans SMEP ni SMAP) | A | [a01_boot_qemu64.py](qemu/a01_boot_qemu64.py) | aucun |
| F79 | transverse | Démarrage et fonctionnement sur matériel réel | M | aucun | MT-02, MT-18 |
| F80 | transverse | Endurance : exécution prolongée sans fuite ni gel | M | aucun | MT-17 |
| F81 | transverse | Cohérence linguistique et typographique de l'interface | M | aucun | MT-22 |
| F82 | transverse | Reproductibilité de la construction sur une machine propre | AM | [mkimage.py](../tools/mkimage.py)<br>[build-toolchain.sh](../tools/build-toolchain.sh)<br>[test_mkimage.py](tools/test_mkimage.py) | MT-24 |
| F83 | apk | Archive ZIP hostile et décompression DEFLATE | A | [test_zip.c](host/d01/test_zip.c)<br>[test_inflate.c](host/d01/test_inflate.c)<br>[test_zip_fuzz.c](host/d01/test_zip_fuzz.c)<br>[test_inflate_fuzz.c](host/d01/test_inflate_fuzz.c) | aucun |
| F84 | apk | XML binaire, table de chaînes et ressources d'un APK | A | [test_respool.c](host/d02/test_respool.c)<br>[test_axml.c](host/d02/test_axml.c)<br>[test_axml_refus.c](host/d02/test_axml_refus.c)<br>[test_arsc.c](host/d02/test_arsc.c)<br>[test_fuzz.c](host/d02/test_fuzz.c) | aucun |
| F85 | apk | Conteneur DEX hostile | A | [test_open.c](host/d03/test_open.c)<br>[test_class.c](host/d03/test_class.c)<br>[test_code.c](host/d03/test_code.c)<br>[test_hostile.c](host/d03/test_hostile.c)<br>[test_fuzz.c](host/d03/test_fuzz.c)<br>[test_croise_dex.c](host/d00/test_croise_dex.c) | aucun |
| F86 | apk | Décodage et vérification structurelle du bytecode | A | [test_table.c](host/d04/test_table.c)<br>[test_decode.c](host/d04/test_decode.c)<br>[test_refus.c](host/d04/test_refus.c)<br>[test_verify.c](host/d04/test_verify.c)<br>[test_fuzz.c](host/d04/test_fuzz.c)<br>[test_croise_opcodes.py](tools/test_croise_opcodes.py) | aucun |
| F87 | apk | Clé publique X.509 et vérification RSA PKCS#1 v1.5 | A | [test_vec.c](host/d05/test_vec.c)<br>[test_fuzz.c](host/d05/test_fuzz.c) | aucun |
| F88 | apk | Signature v2 d'un APK, refus et manifeste (Signature RSA PKCS#1 v1.5 SHA-256 seulement, un signataire.) | A | [test_verify.c](host/d07/test_verify.c)<br>[test_open.c](host/d07/test_open.c)<br>[test_manifest.c](host/d07/test_manifest.c)<br>[test_fuzz.c](host/d07/test_fuzz.c)<br>[test_croise_gros.c](host/d00/test_croise_gros.c)<br>[test_croise_apk.c](host/d00/test_croise_apk.c) | aucun |
| F89 | apk | Machine virtuelle : classes, objets, tas et ramasse-miettes | A | [test_objets.c](host/d08/test_objets.c)<br>[test_classes.c](host/d08/test_classes.c)<br>[test_dex.c](host/d08/test_dex.c)<br>[test_acces.c](host/d08/test_acces.c)<br>[test_appels.c](host/d08/test_appels.c)<br>[test_gc.c](host/d08/test_gc.c)<br>[test_plafond.c](host/d08/test_plafond.c)<br>[test_malloc.c](host/d08/test_malloc.c)<br>[test_croise_oom.c](host/d00/test_croise_oom.c) | aucun |
| F90 | apk | Interpréteur de bytecode et sémantique Java | A | [test_arith.c](host/d09/test_arith.c)<br>[test_div.c](host/d09/test_div.c)<br>[test_conv.c](host/d09/test_conv.c)<br>[test_float.c](host/d09/test_float.c)<br>[test_exc.c](host/d09/test_exc.c)<br>[test_pile.c](host/d09/test_pile.c)<br>[test_natif.c](host/d09/test_natif.c)<br>[test_croise_vm.c](host/d00/test_croise_vm.c)<br>[test_essais.c](host/d11/test_essais.c) | aucun |
| F91 | apk | Bibliothèque de base et modèle des vues d'une appli (Sous-ensemble d'API : une activité, texte, bouton, empilement, journal.) | A | [test_lang.c](host/d11/test_lang.c)<br>[test_texte.c](host/d11/test_texte.c)<br>[test_tableaux.c](host/d11/test_tableaux.c)<br>[test_vues.c](host/d11/test_vues.c)<br>[test_fautes.c](host/d11/test_fautes.c)<br>[test_bonjour.c](host/d11/test_bonjour.c) | aucun |
| F92 | apk | Registre des paquets et installation d'un APK sur /data | A | [test_install.c](host/d10/test_install.c)<br>[test_panne.c](host/d10/test_panne.c)<br>[test_registre.c](host/d10/test_registre.c)<br>[test_champs.c](host/d10/test_champs.c)<br>[test_fuzz.c](host/d10/test_fuzz.c)<br>[test_fs.c](host/d12/test_fs.c)<br>[test_fs_panne.c](host/d12/test_fs_panne.c)<br>[test_inspect.c](host/d12/test_inspect.c)<br>[int_apk_installation.py](qemu/int_apk_installation.py) | aucun |
| F93 | apk | Appli APK lancée du menu Démarrer dans une fenêtre, clics, refus affiché | A | [int_apk_menu.py](qemu/int_apk_menu.py)<br>[int_apk_installation.py](qemu/int_apk_installation.py)<br>[test_lay.c](host/d12/test_lay.c)<br>[test_lay_hostile.c](host/d12/test_lay_hostile.c)<br>[test_programmes_arg.c](host/a20/test_programmes_arg.c)<br>[test_runcmd_apk.c](host/a20/test_runcmd_apk.c) | aucun |
| F94 | apk | Outils de l'hôte : assembleur DEX, fabrication et signature d'APK (Outils et lecteurs écrits séparément depuis les mêmes spécifications.) | A | [test_dexasm.py](tools/test_dexasm.py)<br>[test_mkapk.py](tools/test_mkapk.py)<br>[test_croise_opcodes.py](tools/test_croise_opcodes.py) | aucun |
| F95 | a17 | Verre : flou, saturation, teinte, reflet, liseré, repli opaque et bornes (Ombres de fenêtres non faites, flou du fond d'écran seulement (pas des fenêtres dessous).) | A | [test_glass.c](host/a17/test_glass.c)<br>[test_glass_blur.c](host/a17/test_glass_blur.c)<br>[test_glass_bounds.c](host/a17/test_glass_bounds.c)<br>[test_glass_edge.c](host/a17/test_glass_edge.c)<br>[test_glass_robust.c](host/a17/test_glass_robust.c)<br>[test_glass_robust_band.c](host/a17/test_glass_robust_band.c)<br>[test_glass_robust_corner.c](host/a17/test_glass_robust_corner.c)<br>[test_glass_robust_geo.c](host/a17/test_glass_robust_geo.c)<br>[test_glass_robust_lg.c](host/a17/test_glass_robust_lg.c)<br>[test_glass_robust_zone.c](host/a17/test_glass_robust_zone.c) | aucun |
| F96 | a17 | Palette clair et sombre : jetons, accesseurs de couleur et contrastes (Aucune appli ne bascule le thème : le sombre n'est exercé que par les tests.) | A | [test_colors_ext.c](host/a17/test_colors_ext.c)<br>[test_contrast_bar.c](host/a17/test_contrast_bar.c)<br>[test_contrast_win.c](host/a17/test_contrast_win.c)<br>[test_contrast_fonds.c](host/a20/test_contrast_fonds.c)<br>[test_contrast_plates.c](host/a20/test_contrast_plates.c)<br>[test_uicolor.c](host/a20/test_uicolor.c) | aucun |
| F97 | a17 | Contrôles luna : taille des cases, anneau de focus, dégradé du bouton, encre, symétrie du verre | A | [test_ctrl_bornes.c](host/a17/test_ctrl_bornes.c)<br>[test_ctrl_degrade.c](host/a17/test_ctrl_degrade.c)<br>[test_ctrl_glass.c](host/a17/test_ctrl_glass.c)<br>[test_ctrl_ink.c](host/a17/test_ctrl_ink.c)<br>[test_glass.c](host/a19/test_glass.c)<br>[test_glass_focus.c](host/a19/test_glass_focus.c)<br>[test_glass_font.c](host/a19/test_glass_font.c)<br>[test_ring.c](host/a19/test_ring.c)<br>[test_ring_ctl.c](host/a19/test_ring_ctl.c)<br>[test_ring_face.c](host/a19/test_ring_face.c) | aucun |
| F98 | a17 | Lanceur : disposition de 540 px, zones cliquables, dessin et activation (Recherche factice, quatre tuiles au plus, pas de Récents ni de Toutes les apps.) | A | [test_launcher.c](host/a17/test_launcher.c)<br>[test_launcher_draw.c](host/a17/test_launcher_draw.c)<br>[test_launcher_hit.c](host/a17/test_launcher_hit.c)<br>[test_sm_layout.c](host/a20/test_sm_layout.c)<br>[test_sm_rect.c](host/a20/test_sm_rect.c)<br>[test_sm_activation.c](host/a20/test_sm_activation.c) | aucun |
| F99 | a17 | Fond voile, rendus du bureau, de la connexion et du démarrage (galeries PNG) | A | [test_wallpaper.c](host/a17/test_wallpaper.c)<br>[test_desktop.c](host/a17/test_desktop.c)<br>[test_gallery.c](host/a17/test_gallery.c)<br>[test_glass_gallery.c](host/a17/test_glass_gallery.c) | aucun |
| F100 | a16 | Polices Instrument Sans et JetBrains Mono : génération reproductible, jeu de caractères, budget (Rendu 1 bit, donc moins fin que la maquette.) | A | [test_genfont.py](host/a16/test_genfont.py)<br>[test_font_charset.c](host/a16/test_font_charset.c)<br>[test_font_cover.c](host/a16/test_font_cover.c)<br>[test_font_data.c](host/a16/test_font_data.c)<br>[test_font_budget.c](host/a16/test_font_budget.c) | aucun |
| F101 | a18 | Verre dans le serveur de fenêtres : bandes sales, équivalence incrémental et complet, échec d'allocation | A | [test_core_glass.c](host/a18/test_core_glass.c)<br>[test_core_glass_chain.c](host/a18/test_core_glass_chain.c)<br>[test_core_glass_eq.c](host/a18/test_core_glass_eq.c)<br>[test_srv_glass.c](host/a18/test_srv_glass.c)<br>[test_srv_glass_ext.c](host/a18/test_srv_glass_ext.c) | aucun |
| F102 | a20 | Connexion : mise en page de la maquette, puces, messages et flux invalides | A | [test_logon_layout.c](host/a20/test_logon_layout.c)<br>[test_logon_layout_mockup.c](host/a20/test_logon_layout_mockup.c)<br>[test_logon_layout_ring.c](host/a20/test_logon_layout_ring.c)<br>[test_logon_text.c](host/a20/test_logon_text.c)<br>[test_logon_flow_invalid.c](host/a20/test_logon_flow_invalid.c) | aucun |
| F103 | a20 | Bureau : grille d'icônes, zones, sélection au clavier, double-clic et fenêtre Bonjour | A | [test_desktop.c](host/a20/test_desktop.c)<br>[test_desktop_zone.c](host/a20/test_desktop_zone.c)<br>[test_dblclick.c](host/a20/test_dblclick.c)<br>[test_hello_layout.c](host/a20/test_hello_layout.c) | aucun |
