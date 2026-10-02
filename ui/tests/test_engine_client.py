"""Tests for gambit_ui.engine_client. Phase 1 holds only an import check; the real tests come in Phase 3."""
import importlib


def test_module_imports():
    importlib.import_module("gambit_ui.engine_client")
