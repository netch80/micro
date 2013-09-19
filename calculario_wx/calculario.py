#!/usr/bin/env python
import wx

ID_MODE_STACK                   = 12000
ID_MODE_INFIX                   = 12001


op_prios = {
    '(': 0,     ## will be handled only with ')'
    '+': 1,
    '-': 1,
    '*': 2,
    '/': 2,
}


def comparePrio(op1, op2):
    return op_prios[op1] - op_prios[op2]


class MainWindow(wx.Frame):

    current_value = None
    stack = None
    op_stack = None
    in_input = False
    must_auto_dup = False
    mode_infix = False
    memory_value = None

    def __init__(self):

        wx.Frame.__init__(self, None, wx.ID_ANY, "Calculario")

        self.current_value = ''
        self.memory_value = 0.0
        self.stack = []
        self.op_stack = []

        number_menu = wx.Menu()
        menu_item_exit = number_menu.Append(wx.ID_EXIT,
                "E&xit" , " Terminate the program")
        self.Bind(wx.EVT_MENU, self.OnExit, menu_item_exit)
        mode_menu = wx.Menu()
        self.menu_item_mode_stack = mode_menu.AppendCheckItem(ID_MODE_STACK,
                "&Stack", " Switch to stack (postfix) mode")
        self.menu_item_mode_infix = mode_menu.AppendCheckItem(ID_MODE_INFIX,
                "&Infix", " Switch to infix mode")
        self.menu_item_mode_stack.Check()
        self.Bind(wx.EVT_MENU,
                  lambda _e: self.setInfixMode(True),
                  self.menu_item_mode_infix)
        self.Bind(wx.EVT_MENU,
                  lambda _e: self.setInfixMode(False),
                  self.menu_item_mode_stack)
        menuBar = wx.MenuBar()
        menuBar.Append(number_menu, "&Number")
        menuBar.Append(mode_menu, "&Mode")
        self.SetMenuBar(menuBar)

        self.buttons = {}
        self.top_sizer = wx.BoxSizer(wx.VERTICAL)
        self.top_buttons_sizer = wx.BoxSizer(wx.HORIZONTAL)
        self.st_in_input = wx.StaticText(self, label = ' -RN- ')
        self.top_buttons_sizer.Add(self.st_in_input)
        self.st_stack_size = wx.StaticText(self, label = ' =0= ')
        self.top_buttons_sizer.Add(self.st_stack_size)
        self.st_memory = wx.StaticText(self, label = ' ')
        self.top_buttons_sizer.Add(self.st_memory)
        self.buttons_sizer = wx.GridSizer(rows = 5, cols = 5,
                vgap = 10, hgap = 10)
        for bkey in ('bs', 'x1', 'x2', 'MC', 'C',
                      '7', '8', '9', 'M+', 'MR',
                      '4', '5', '6', '*', '/',
                      '1', '2', '3', '+', '-',
                      '0', '.', 'neg', 'dup', 'CE'):
            btext = {
                    'neg': '+/-',
                    'dup': 'DUP',
                    'bs': '00->0',
                    'x1': 'DROP',
                    'x2': 'SWAP',
                }.get(bkey, bkey)
            button = wx.Button(self, label = btext)
            self.buttons[bkey] = button
            self.buttons_sizer.Add(button)
        for bkey in ('0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.'):
            self.Bind(wx.EVT_BUTTON,
                      lambda _e, bkey=bkey:
                            self.onInputButton(bkey),
                      self.buttons[bkey])
        for bkey in ('+', '-', '*', '/'):
            self.Bind(wx.EVT_BUTTON,
                      lambda _e, bkey=bkey:
                            self.btTwoArgumentOperator(bkey),
                      self.buttons[bkey])
        self.Bind(wx.EVT_BUTTON, lambda _e: self.onDupButton(),
                self.buttons['dup'])
        self.Bind(wx.EVT_BUTTON, lambda _e: self.onNegateButton(),
                self.buttons['neg'])
        self.Bind(wx.EVT_BUTTON, lambda _e: self.onBackspaceButton(),
                self.buttons['bs'])
        self.Bind(wx.EVT_BUTTON, lambda _e: self.onClearButton(),
                self.buttons['C'])
        self.Bind(wx.EVT_BUTTON, lambda _e: self.onX1Button(),
                self.buttons['x1'])
        self.Bind(wx.EVT_BUTTON, lambda _e: self.onX2Button(),
                self.buttons['x2'])
        self.Bind(wx.EVT_BUTTON, lambda _e: self.onMemAddButton(),
                self.buttons['M+'])
        self.Bind(wx.EVT_BUTTON, lambda _e: self.onMemReadButton(),
                self.buttons['MR'])
        self.Bind(wx.EVT_BUTTON, lambda _e: self.onMemClearButton(),
                self.buttons['MC'])

        self.indicator = wx.TextCtrl(self, value = '0',
                style = wx.SIMPLE_BORDER)
        self.top_sizer.Add(self.top_buttons_sizer, flag = wx.EXPAND)
        self.top_sizer.Add(wx.StaticLine(self), flag = wx.EXPAND)
        self.top_sizer.Add(self.indicator, flag = wx.EXPAND)
        self.top_sizer.Add(wx.StaticLine(self), flag = wx.EXPAND)
        self.top_sizer.Add(self.buttons_sizer, flag = wx.EXPAND)
        self.SetSizer(self.top_sizer)
        self.SetAutoLayout(1)
        self.top_sizer.Fit(self)

    def setInfixMode(self, to_infix):
        self.mode_infix = to_infix
        self.menu_item_mode_infix.Check(to_infix)
        self.menu_item_mode_stack.Check(not to_infix)
        self.buttons['dup'].SetLabel('=' if to_infix else 'DUP')
        self.buttons['x1'].SetLabel('(' if to_infix else '')
        self.buttons['x2'].SetLabel(')' if to_infix else '')
        self.stack = []
        self.op_stack = []
        self.in_input = False
        self.need_auto_dup = False
        self.reIndicate()

    def OnExit(self, _e):
        self.Close(True)

    def onInputButton(self, bkey):
        if not self.in_input:
            if self.must_auto_dup and self.current_value:
                self.stack.append(self.current_value)
            self.current_value = ''
            self.in_input = True
            self.must_auto_dup = False
        if bkey == '.' and '.' in self.current_value:
            ## We simply ignore such case
            return
        self.current_value += bkey
        self.reIndicate()

    def btTwoArgumentOperator(self, bkey):
        if self.mode_infix:
            return self.btTwoArgumentOperator_Infix(bkey)
        else:
            return self.btTwoArgumentOperator_Stack(bkey)

    def btTwoArgumentOperator_Stack(self, bkey):
        if not self.stack:
            return
        self._applyOperator(bkey)
        self.in_input = False
        self.must_auto_dup = True
        self.reIndicate()

    def _popOperator(self):
        ## Infix mode only
        op = self.op_stack.pop()
        return self._applyOperator(op)

    def _applyOperator(self, op):
        a = float(self.stack.pop())
        c = float(self.current_value or '0')
        if op == '+':
            v = a + c
        elif op == '-':
            v = a - c
        elif op == '*':
            v = a * c
        elif op == '/':
            v = a / c
        self.current_value = str(v)

    def btTwoArgumentOperator_Infix(self, bkey):
        ## This shall:
        ## 1) Check the operation stack against all most (or same)
        ## prioritized operators, and, if found, pop them and apply;
        ## 2) push the current value onto the data stack, and
        ## the operator - onto the operator stack
        while self.op_stack and self.stack and \
                comparePrio(bkey, self.op_stack[-1]) <= 0:
            self._popOperator()
        self.stack.append(self.current_value or '0')
        self.op_stack.append(bkey)
        self.in_input = False
        self.must_auto_dup = False
        self.reIndicate()

    def onDupButton(self):
        if self.mode_infix:
            return self.onEqualButton()
        self.in_input = False
        self.must_auto_dup = False
        self.stack.append(self.current_value or '0')
        self.reIndicate()

    def onEqualButton(self):
        #- print '__: onEqualButton: stack=%r op_stack=%r' % \
        #-     (self.stack, self.op_stack,)
        while self.stack and self.op_stack:
            self._popOperator()
        self.reIndicate()

    def onX1Button(self):
        if self.mode_infix:
            self.op_stack.append('(')
        else:
            if self.stack:
                self.current_value = self.stack.pop()
            else:
                self.current_value = ''
            self.in_input = False
            self.must_auto_dup = False
            self.reIndicate()

    def onX2Button(self):
        if self.mode_infix:
            while self.op_stack and self.op_stack[-1] != '(':
                self._popOperator()
            self.op_stack.pop()
            self.reIndicate()
        else:
            if self.stack:
                t = self.stack.pop()
                self.stack.append(self.current_value or '0')
                self.current_value = t
            self.in_input = False
            self.must_auto_dup = False
            self.reIndicate()

    def onNegateButton(self):
        self.in_input = False
        self.must_auto_dup = not self.mode_infix
        if self.current_value:
            self.current_value = str(-float(self.current_value))
            self.reIndicate()

    def onBackspaceButton(self):
        self.in_input = True
        self.must_auto_dup = False
        if self.current_value:
            self.current_value = self.current_value[:-1]
            self.reIndicate()

    def onMemAddButton(self):
        self.memory_value += float(self.current_value or '0')
        self.reIndicate()

    def onMemReadButton(self):
        if self.must_auto_dup and not self.mode_infix:
            self.stack.append(self.current_value or '0')
        self.current_value = str(self.memory_value)
        self.in_input = False
        self.must_auto_dup = not self.mode_infix
        self.reIndicate()

    def onMemClearButton(self):
        self.memory_value = 0
        self.reIndicate()

    def onClearButton(self):
        self.current_value = ''
        self.stack = []
        self.op_stack = []
        self.in_input = False
        self.must_auto_dup = False
        self.reIndicate()

    def reIndicate(self):
        v = self.current_value or '0'
        self.indicator.SetValue(v)
        self.st_in_input.SetLabel(' -%s%s- ' % (
                'I' if self.in_input else 'R',
                'A' if self.must_auto_dup else 'N',))
        self.st_stack_size.SetLabel(' =%d= ' % (len(self.stack),))
        self.st_memory.SetLabel('M' if self.memory_value else ' ')


if __name__ == '__main__':
    app = wx.App(False)         ## don't redirect stdout
    frame = MainWindow()
    frame.Show(True)
    app.MainLoop()
