#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
FST - Hackers Tchad
Auteur : Hackers Tchad
Outil de démonstration avancé avec ASCII art et barre de progression.
Compatible Termux Android & terminaux standards.
"""

import re
import sys
import time
import shutil
import os


# =============================================================================
# COULEURS / STYLES ANSI
# =============================================================================
class Colors:
    RESET = "\033[0m"
    BOLD = "\033[1m"
    DIM = "\033[2m"
    UNDERLINE = "\033[4m"
    BLINK = "\033[5m"
    GREEN = "\033[32m"
    LIGHT_GREEN = "\033[92m"
    DARK_GREEN = "\033[38;5;22m"
    CYAN = "\033[36m"
    LIGHT_CYAN = "\033[96m"
    RED = "\033[31m"
    YELLOW = "\033[33m"
    MAGENTA = "\033[35m"
    WHITE = "\033[37m"
    GRAY = "\033[90m"
    BG_BLACK = "\033[40m"
    BG_GREEN = "\033[42m"


# =============================================================================
# ASCII ART - BANNIÈRE FST
# =============================================================================
# ---------------------------------------------------------------------------
# EMPLACEMENT ASCII ART FST
# Remplace les lignes entre les marqueurs ci-dessous par ton image ASCII.
# Les balises {green}, {cyan}, {bold}, etc. seront formatées automatiquement.
# ---------------------------------------------------------------------------
ASCII_BANNER = r"""
{green}╔══════════════════════════════════════════════════╗{reset}
{green}║                                                  ║{reset}
{green}║             [ PLACE TON ASCII ICI ]              ║{reset}
{green}║                                                  ║{reset}
{green}╚══════════════════════════════════════════════════╝{reset}
{cyan}╔══════════════════════════════════════════════════╗{reset}
{cyan}║{reset}   {bold}{white}FST - Advanced Terminal Utility{reset}              {cyan}║{reset}
{cyan}║{reset}   {yellow}Auteur : {reset}{light_green}Hackers Tchad{reset}                        {cyan}║{reset}
{cyan}║{reset}   {gray}Compatible Termux / Linux / macOS{reset}            {cyan}║{reset}
{cyan}╚══════════════════════════════════════════════════╝{reset}
"""


def center_text(text: str, width: int = None) -> str:
    """Centre un texte multi-lignes dans la largeur du terminal."""
    if width is None:
        width = shutil.get_terminal_size((80, 24)).columns
    lines = text.splitlines()
    centered = []
    for line in lines:
        visible = strip_ansi(line)
        pad = max(0, (width - len(visible)) // 2)
        centered.append(" " * pad + line)
    return "\n".join(centered)


def strip_ansi(text: str) -> str:
    """Supprime les codes ANSI pour calculer la vraie longueur visible."""
    ansi = re.compile(r"\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])")
    return ansi.sub("", text)


def print_banner():
    """Affiche la bannière ASCII centrée et colorée."""
    width = shutil.get_terminal_size((80, 24)).columns
    banner = ASCII_BANNER.format(
        green=Colors.DARK_GREEN + Colors.BOLD,
        cyan=Colors.CYAN,
        reset=Colors.RESET,
        bold=Colors.BOLD,
        white=Colors.WHITE,
        yellow=Colors.YELLOW,
        light_green=Colors.LIGHT_GREEN,
        gray=Colors.GRAY,
    )
    print(center_text(banner, width))


# =============================================================================
# BARRE DE PROGRESSION PERSONNALISÉE
# =============================================================================
def progress_bar(label: str, duration: float = 2.5, steps: int = 40):
    """Affiche une barre de progression style hacker dans le terminal."""
    width = shutil.get_terminal_size((80, 24)).columns
    bar_width = min(50, max(20, width - 30))
    print(f"\n{Colors.CYAN}[*] {label}{Colors.RESET}")
    for i in range(steps + 1):
        pct = int((i / steps) * 100)
        filled = int((i / steps) * bar_width)
        empty = bar_width - filled
        bar = (
            Colors.BG_GREEN + " " * filled + Colors.RESET
            if filled > 0
            else ""
        )
        blank = Colors.GRAY + "─" * empty + Colors.RESET
        sys.stdout.write(
            f"\r{Colors.GREEN}  [{bar}{blank}] {pct:3d}%{Colors.RESET}"
        )
        sys.stdout.flush()
        time.sleep(duration / steps)
    print(f"\n{Colors.LIGHT_GREEN}[✔] {label} terminé.{Colors.RESET}\n")


# =============================================================================
# ANIMATION DE TYPAGE HACKER
# =============================================================================
def typewriter(text: str, delay: float = 0.03, color: str = Colors.LIGHT_GREEN):
    """Affiche un texte avec effet machine à écrire."""
    sys.stdout.write(color)
    for char in text:
        sys.stdout.write(char)
        sys.stdout.flush()
        time.sleep(delay)
    sys.stdout.write(Colors.RESET + "\n")


# =============================================================================
# SIMULATION DE COMMANDE SELON LE MOT SAISI
# =============================================================================
def execute_command(command_word: str):
    """Simule l'exécution d'une commande après validation."""
    width = shutil.get_terminal_size((80, 24)).columns
    print(center_text(f"{Colors.CYAN}╔{'═' * (width - 2)}╗{Colors.RESET}", width))
    header = f"{Colors.CYAN}║{Colors.RESET} {Colors.BOLD}{Colors.WHITE}EXÉCUTION DU MODULE : {command_word.upper()}{Colors.RESET}"
    pad = width - len(strip_ansi(header)) - 1
    print(header + " " * pad + f"{Colors.CYAN}║{Colors.RESET}")
    print(center_text(f"{Colors.CYAN}╠{'═' * (width - 2)}╣{Colors.RESET}", width))

    progress_bar("Initialisation du module", duration=1.5)
    progress_bar("Connexion sécurisée", duration=2.0)
    progress_bar(f"Exécution de {command_word}", duration=2.5)

    typewriter(
        f"  [INFO] Module '{command_word}' exécuté avec succès.",
        delay=0.02,
        color=Colors.LIGHT_GREEN,
    )
    typewriter(
        "  [INFO] Aucune donnée sensible n'a été modifiée.",
        delay=0.02,
        color=Colors.GRAY,
    )
    print(
        center_text(
            f"{Colors.CYAN}╚{'═' * (width - 2)}╝{Colors.RESET}",
            width,
        )
    )


# =============================================================================
# MENU INTERACTIF
# =============================================================================
def main():
    """Point d'entrée principal."""
    os.system("clear" if os.name == "posix" else "cls")
    print_banner()

    typewriter(
        "Bienvenue dans l'utilitaire FST développé par Hackers Tchad.",
        delay=0.02,
        color=Colors.LIGHT_CYAN,
    )
    print()

    prompt_text = f"{Colors.YELLOW}[?]{Colors.RESET} Entrez votre commande : {Colors.GREEN}"

    try:
        user_command = input(prompt_text)
    except (EOFError, KeyboardInterrupt):
        print(f"\n{Colors.RED}[✘] Interrompu.{Colors.RESET}")
        sys.exit(1)

    print(Colors.RESET)

    if user_command.strip():
        print(f"{Colors.LIGHT_GREEN}[✔] Commande acceptée : {user_command.strip()}{Colors.RESET}")
        progress_bar("Exécution en cours", duration=10.0)
        execute_command(user_command.strip())
    else:
        print(f"{Colors.RED}[✘] Aucune commande saisie.{Colors.RESET}")
        sys.exit(1)

    # Footer
    print()
    footer = f"{Colors.DIM}{Colors.GREEN}Hackers Tchad © 2024 - FST Utility{Colors.RESET}"
    print(center_text(footer))


if __name__ == "__main__":
    main()
