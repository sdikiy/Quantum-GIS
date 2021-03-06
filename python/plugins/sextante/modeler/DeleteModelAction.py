from sextante.gui.ContextAction import ContextAction
from sextante.modeler.ModelerAlgorithm import ModelerAlgorithm
import os
from PyQt4 import QtGui

class DeleteModelAction(ContextAction):

    def __init__(self):
        self.name="Delete model"

    def isEnabled(self):
        return isinstance(self.alg, ModelerAlgorithm)

    def execute(self):
        reply = QtGui.QMessageBox.question(None, 'Confirmation',
                            "Are you sure you want to delete this model?", QtGui.QMessageBox.Yes |
                            QtGui.QMessageBox.No, QtGui.QMessageBox.No)
        if reply == QtGui.QMessageBox.Yes:
            os.remove(self.alg.descriptionFile)
            self.toolbox.updateTree()