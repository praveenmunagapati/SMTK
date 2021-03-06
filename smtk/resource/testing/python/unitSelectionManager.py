#=============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=============================================================================
import smtk
import smtk.attribute
import smtk.model
import smtk.bridge.polygon
import smtk.testing
from smtk.simple import GetActiveSession, SetActiveSession, LoadSMTKModel

import sys
import uuid

eventCount = 0


class TestSelectionManager(smtk.testing.TestCase):

    def setUp(self):
        self.selnMgr = smtk.resource.SelectionManager.create()

    def loadTestData(self):
        import os
        self.mgr = smtk.model.Manager.create()
        SetActiveSession(self.mgr.createSession('polygon'))
        self.assertIsNotNone(
            GetActiveSession(), 'Could not create polygon session.')
        fpath = [smtk.testing.DATA_DIR, 'model',
                 '2d', 'smtk', 'epic-trex-drummer.smtk']
        self.models = LoadSMTKModel(os.path.join(*fpath))
        self.model = smtk.model.Model(self.models[0])

    def testSelectionValues(self):
        mgr = self.selnMgr
        self.assertFalse(mgr.registerSelectionValue(
            'unselected', 0), 'Should not be able to register value of 0.')
        self.assertTrue(mgr.registerSelectionValue(
            'hover', 1), 'Should be able to register non-zero value.')
        self.assertFalse(mgr.registerSelectionValue(
            'barf', 1), 'Should not be able to re-register value.')
        self.assertTrue(mgr.registerSelectionValue('barfo', 1, False),
                        'Should be able to re-register value explicitly.')
        self.assertTrue(mgr.registerSelectionValue(
            'selection', 2), 'Should be able to register multiple values.')
        self.assertTrue(mgr.registerSelectionValue('naughty', 2, False),
                        'Should be able to re-register value explicitly.')

        self.assertEqual(mgr.selectionValueFromLabel(
            'hover'), 1, 'Registered values do not match.')
        self.assertEqual(mgr.selectionValueFromLabel(
            'barfo'), 1, 'Registered values do not match.')
        self.assertEqual(mgr.selectionValueFromLabel(
            'selection'), 2, 'Registered values do not match.')
        self.assertEqual(mgr.selectionValueFromLabel(
            'naughty'), 2, 'Registered values do not match.')
        svl = {u'selection': 2L, u'naughty': 2L, u'hover': 1L, u'barfo': 1L}
        self.assertEqual(mgr.selectionValueLabels(),
                         svl, 'Unexpected selection value labels.')
        self.assertEqual(mgr.findOrCreateLabeledValue(
            'selection'), 2, 'Did not find pre-existing labeled value.')
        selnLbl = 'norkit'
        selnVal = mgr.findOrCreateLabeledValue(selnLbl)
        svl[selnLbl] = selnVal

        self.assertTrue(mgr.unregisterSelectionValue(
            'naughty'), 'Could not unregister extant selection value.')
        self.assertFalse(mgr.unregisterSelectionValue(
            'naughty'), 'Could unregister extinct selection value.')
        self.assertFalse(mgr.unregisterSelectionValue(
            'nutty'), 'Could unregister non-existent selection value.')
        del svl['naughty']
        for kk, vv in svl.items():
            self.assertEqual(mgr.selectionValueFromLabel(
                kk), vv, 'Failed to look up selection value from label.')
            self.assertTrue(mgr.unregisterSelectionValue(
                kk), 'Failed to unregister a selection value label.')

    def testSelectionSources(self):
        mgr = self.selnMgr
        self.assertTrue(mgr.registerSelectionSource(
            'foo'), 'Could not register selection source.')
        self.assertFalse(mgr.registerSelectionSource(
            'foo'), 'Could re-register selection source.')
        self.assertTrue(mgr.unregisterSelectionSource(
            'foo'), 'Could not unregister extant selection source.')
        self.assertFalse(mgr.unregisterSelectionSource(
            'foo'), 'Could unregister extinct selection.')

    def testSelectionModificationFilterListen(self):
        self.loadTestData()
        mgr = self.selnMgr

        def listener(src, smgr):
            global eventCount
            print('Selection updated by %s' % src)
            eventCount += 1

        # Test that listener is called at proper times:
        handle = mgr.listenToSelectionEvents(listener, True)
        print('Selection event listener-handle %d' % handle)
        self.assertGreaterEqual(
            handle, 0, 'Failed to register selection listener.')
        expectedEventCount = 1
        self.assertEqual(eventCount, expectedEventCount,
                         'Selection listener was not called immediately.')
        comp = self.mgr.find(self.model.entity())
        selnVal = mgr.findOrCreateLabeledValue('selection')
        selnSrc = 'testSelectionModificationListen'
        mgr.registerSelectionSource(selnSrc)
        mgr.registerSelectionValue('selection', selnVal)

        # Test selection modification and insure listener is called properly:
        mgr.modifySelection(
            [comp, ], selnSrc, selnVal, smtk.resource.SelectionAction.DEFAULT)
        expectedEventCount += 1
        self.assertEqual(eventCount, expectedEventCount,
                         'Selection listener was not called upon change.')
        mgr.modifySelection(
            [comp, ], selnSrc, selnVal, smtk.resource.SelectionAction.FILTERED_ADD)
        self.assertEqual(eventCount, expectedEventCount,
                         'Selection listener was called when there should be no event.')
        mgr.modifySelection(
            [], selnSrc, selnVal, smtk.resource.SelectionAction.DEFAULT)
        expectedEventCount += 1
        self.assertEqual(eventCount, expectedEventCount,
                         'Selection listener was not called upon modification.')

        # Test that filtering works
        def allPassFilter(comp, lvl, sugg):
            return True
        mgr.modifySelection(
            [comp, ], selnSrc, selnVal, smtk.resource.SelectionAction.DEFAULT)
        expectedEventCount += 1
        mgr.setFilter(allPassFilter)
        self.assertEqual(eventCount, expectedEventCount,
                         'Selection listener was called upon no-op filter pass.')

        def nonePassFilter(comp, lvl, sugg):
            return False
        mgr.setFilter(nonePassFilter)
        expectedEventCount += 1
        self.assertEqual(eventCount, expectedEventCount,
                         'Selection listener was not called upon filter pass.')

        # We cannot test filters that suggest new values in Python
        # because pybind copies the suggestions map by value rather
        # than passing a reference:
        # def suggestFilter(comp, lvl, sugg):
        #     model = smtk.model.Model(comp.modelResource(), comp.id())
        #     for cell in model.cells():
        #         sugg[cell] = lvl
        #     return False
        # mgr.setFilter(suggestFilter)
        # mgr.modifySelection([comp,], selnSrc, selnVal, smtk.resource.SelectionAction.DEFAULT)
        # expectedEventCount += 1
        # print(mgr.currentSelection())
        # self.assertEqual(eventCount, expectedEventCount, 'Selection listener
        # was not called upon suggestion pass.')

        self.assertTrue(mgr.unlisten(handle), 'Could not unregister listener')
        self.assertFalse(
            mgr.unlisten(handle), 'Could double-unregister listener')

        # Test enumeration of selection
        model = smtk.model.Model(comp.modelResource(), comp.id())
        cellComps = [self.mgr.find(cell.entity()) for cell in model.cells()]
        mgr.setFilter(None)
        mgr.modifySelection(
            cellComps, selnSrc, selnVal, smtk.resource.SelectionAction.DEFAULT)
        dd = {kk.id(): vv for kk, vv in mgr.currentSelection().items()}
        print(
            ''.join(['  ', '\n  '.join(['%s: %d' % (str(kk), vv) for kk, vv in dd.items()])]))
        ddExpected = {
            uuid.UUID('e2af5e59-6c2f-4cfe-bdd5-8e806906de44'): 1L,
            uuid.UUID('6382323f-e8e4-455e-b889-fd1c0dc40be5'): 1L,
            uuid.UUID('3894f798-75a9-4b74-9d53-d93ddbb513d1'): 1L,
            uuid.UUID('77b2dbdd-0a70-47a9-a132-1af51353769c'): 1L,
            uuid.UUID('12e0f395-8c51-4ee3-aafe-7fc108e8f48a'): 1L,
            uuid.UUID('12706f3a-a528-440e-ad02-0eb907d0079a'): 1L,
            uuid.UUID('36ccbcf5-240a-4f99-88ba-c5418fcfef10'): 1L,
            uuid.UUID('3885d08e-c9bb-4d29-aa80-0143e50bea81'): 1L,
            uuid.UUID('44c34ccc-284f-49f4-afd0-8efa59e115f9'): 1L,
            uuid.UUID('e996d852-1b9a-43a5-a98a-e58bad72c207'): 1L,
            uuid.UUID('04fa7479-5d03-4b7c-8fbd-1a3e2341baf6'): 1L,
            uuid.UUID('5ee2ab86-3d5c-4350-82ca-83afdeb425a4'): 1L,
            uuid.UUID('977e9cb5-a657-4b66-bf76-9cfeb9409171'): 1L,
            uuid.UUID('8946acb8-7ad5-4a13-981d-b04881ff9248'): 1L,
            uuid.UUID('fc2b03b4-8591-4796-9968-881d9461e1e6'): 1L,
            uuid.UUID('69504b3f-35a3-41ca-a50d-78c31216f134'): 1L
        }
        self.assertEqual(dd, ddExpected, 'Unexpected selection.')
        global visitSeln
        visitSeln = {}

        def selectionVisitor(comp, lvl):
            global visitSeln
            visitSeln[comp.id()] = lvl
        mgr.visitSelection(selectionVisitor)
        self.assertEqual(
            visitSeln, ddExpected, 'Did not visit selection properly.')

if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
