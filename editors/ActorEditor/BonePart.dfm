object frmBonePart: TfrmBonePart
  Left = 426
  Top = 321
  BorderStyle = bsToolWindow
  Caption = 'Bone Part'
  ClientHeight = 451
  ClientWidth = 441
  Color = 10528425
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  Scaled = False
  OnKeyDown = FormKeyDown
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object RxLabel29: TLabel
    Left = 0
    Top = 19
    Width = 38
    Height = 13
    Caption = 'Part #1:'
  end
  object RxLabel1: TLabel
    Left = 224
    Top = 19
    Width = 38
    Height = 13
    Caption = 'Part #2:'
  end
  object RxLabel2: TLabel
    Left = 0
    Top = 227
    Width = 38
    Height = 13
    Caption = 'Part #3:'
  end
  object RxLabel3: TLabel
    Left = 224
    Top = 227
    Width = 38
    Height = 13
    Caption = 'Part #4:'
  end
  object ebSave: TExtBtn
    Left = 247
    Top = 433
    Width = 96
    Height = 17
    Align = alNone
    BevelShow = False
    Caption = 'Ok'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlack
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    FlatAlwaysEdge = True
    OnClick = ebSaveClick
  end
  object ebCancel: TExtBtn
    Left = 344
    Top = 433
    Width = 96
    Height = 17
    Align = alNone
    BevelShow = False
    Caption = 'Cancel'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlack
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    FlatAlwaysEdge = True
    OnClick = ebCancelClick
  end
  object ExtBtn1: TExtBtn
    Left = 0
    Top = 433
    Width = 100
    Height = 17
    Align = alNone
    BevelShow = False
    Caption = 'Reset To Default'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlack
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    FlatAlwaysEdge = True
    OnClick = ExtBtn1Click
  end
  object lbPart1: TLabel
    Left = 40
    Top = 19
    Width = 30
    Height = 13
    Caption = '(XX B)'
  end
  object lbPart3: TLabel
    Left = 40
    Top = 227
    Width = 30
    Height = 13
    Caption = '(XX B)'
  end
  object lbPart4: TLabel
    Left = 264
    Top = 227
    Width = 30
    Height = 13
    Caption = '(XX B)'
  end
  object lbPart2: TLabel
    Left = 264
    Top = 19
    Width = 30
    Height = 13
    Caption = '(XX B)'
  end
  object Label1: TLabel
    Left = 0
    Top = 3
    Width = 60
    Height = 13
    Caption = 'Total Bones:'
  end
  object lbTotalBones: TLabel
    Left = 71
    Top = 3
    Width = 9
    Height = 13
    Caption = '...'
  end
  object ExtBtn2: TExtBtn
    Left = 180
    Top = 18
    Width = 36
    Height = 17
    Align = alNone
    BevelShow = False
    Caption = 'Clear'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlack
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    FlatAlwaysEdge = True
    OnClick = ebClearClick
  end
  object ExtBtn3: TExtBtn
    Tag = 1
    Left = 404
    Top = 18
    Width = 36
    Height = 17
    Align = alNone
    BevelShow = False
    Caption = 'Clear'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlack
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    FlatAlwaysEdge = True
    OnClick = ebClearClick
  end
  object ExtBtn4: TExtBtn
    Tag = 3
    Left = 404
    Top = 226
    Width = 36
    Height = 17
    Align = alNone
    BevelShow = False
    Caption = 'Clear'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlack
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    FlatAlwaysEdge = True
    OnClick = ebClearClick
  end
  object ExtBtn5: TExtBtn
    Tag = 2
    Left = 180
    Top = 226
    Width = 36
    Height = 17
    Align = alNone
    BevelShow = False
    Caption = 'Clear'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlack
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    FlatAlwaysEdge = True
    OnClick = ebClearClick
  end
  object ebSaveTo: TExtBtn
    Left = 103
    Top = 434
    Width = 66
    Height = 17
    Align = alNone
    BevelShow = False
    Caption = 'Save to'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlack
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    FlatAlwaysEdge = True
    OnClick = ebSaveToClick
  end
  object ebLoadFrom: TExtBtn
    Left = 167
    Top = 434
    Width = 66
    Height = 17
    Align = alNone
    BevelShow = False
    Caption = 'Load from'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlack
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    FlatAlwaysEdge = True
    OnClick = ebLoadFromClick
  end
  object tvPart1: TElTree
    Left = 0
    Top = 38
    Width = 217
    Height = 185
    AlwaysKeepFocus = True
    DoInplaceEdit = False
    DragAllowed = True
    DrawFocusRect = False
    FocusedSelectColor = 10526880
    FocusedSelectTextColor = clBlack
    HeaderHeight = 19
    HeaderHotTrack = False
    HeaderSections.Data = {F1FFFFFF00000000}
    HorizontalLines = True
    HorzDivLinesColor = 7368816
    HorzScrollBarStyles.Width = 17
    HorzScrollBarStyles.ButtonSize = 16
    HorzScrollBarStyles.UseSystemMetrics = False
    HorzScrollBarStyles.ThemeMode = ttmNone
    ItemIndent = 14
    LineHeight = 16
    HorzScrollTracking = True
    VertScrollTracking = True
    ShowButtons = False
    ShowCheckBoxes = True
    ShowImages = False
    ShowLines = False
    ShowRootButtons = False
    SortMode = smAdd
    StoragePath = '\Tree'
    TabOrder = 0
    Tracking = False
    TrackColor = clBlack
    VertScrollBarStyles.ShowTrackHint = True
    VertScrollBarStyles.Width = 17
    VertScrollBarStyles.ButtonSize = 16
    VertScrollBarStyles.UseSystemMetrics = False
    VertScrollBarStyles.ThemeMode = ttmNone
    ThemeMode = ttmNone
    BkColor = clGray
    OnDragDrop = tvPartDragDrop
    OnDragOver = tvPartDragOver
    OnStartDrag = tvPartStartDrag
  end
  object edPart1Name: TEdit
    Left = 71
    Top = 18
    Width = 107
    Height = 16
    AutoSize = False
    Color = 10526880
    TabOrder = 1
  end
  object tvPart2: TElTree
    Left = 224
    Top = 38
    Width = 217
    Height = 185
    AlwaysKeepFocus = True
    DoInplaceEdit = False
    DragAllowed = True
    DrawFocusRect = False
    FocusedSelectColor = 10526880
    FocusedSelectTextColor = clBlack
    HeaderHeight = 19
    HeaderHotTrack = False
    HeaderSections.Data = {F1FFFFFF00000000}
    HorizontalLines = True
    HorzDivLinesColor = 7368816
    HorzScrollBarStyles.Width = 17
    HorzScrollBarStyles.ButtonSize = 16
    HorzScrollBarStyles.UseSystemMetrics = False
    HorzScrollBarStyles.ThemeMode = ttmNone
    ItemIndent = 14
    LineHeight = 16
    HorzScrollTracking = True
    VertScrollTracking = True
    ShowButtons = False
    ShowCheckBoxes = True
    ShowImages = False
    ShowLines = False
    ShowRootButtons = False
    SortMode = smAdd
    StoragePath = '\Tree'
    TabOrder = 2
    Tracking = False
    TrackColor = clBlack
    VertScrollBarStyles.ShowTrackHint = True
    VertScrollBarStyles.Width = 17
    VertScrollBarStyles.ButtonSize = 16
    VertScrollBarStyles.UseSystemMetrics = False
    VertScrollBarStyles.ThemeMode = ttmNone
    ThemeMode = ttmNone
    BkColor = clGray
    OnDragDrop = tvPartDragDrop
    OnDragOver = tvPartDragOver
    OnStartDrag = tvPartStartDrag
  end
  object edPart2Name: TEdit
    Left = 295
    Top = 18
    Width = 107
    Height = 16
    AutoSize = False
    Color = 10526880
    TabOrder = 3
  end
  object tvPart3: TElTree
    Left = 0
    Top = 246
    Width = 217
    Height = 185
    AlwaysKeepFocus = True
    DoInplaceEdit = False
    DragAllowed = True
    DrawFocusRect = False
    FocusedSelectColor = 10526880
    FocusedSelectTextColor = clBlack
    HeaderHeight = 19
    HeaderHotTrack = False
    HeaderSections.Data = {F1FFFFFF00000000}
    HorizontalLines = True
    HorzDivLinesColor = 7368816
    HorzScrollBarStyles.Width = 17
    HorzScrollBarStyles.ButtonSize = 16
    HorzScrollBarStyles.UseSystemMetrics = False
    HorzScrollBarStyles.ThemeMode = ttmNone
    ItemIndent = 14
    LineHeight = 16
    HorzScrollTracking = True
    VertScrollTracking = True
    ShowButtons = False
    ShowCheckBoxes = True
    ShowImages = False
    ShowLines = False
    ShowRootButtons = False
    SortMode = smAdd
    StoragePath = '\Tree'
    TabOrder = 4
    Tracking = False
    TrackColor = clBlack
    VertScrollBarStyles.ShowTrackHint = True
    VertScrollBarStyles.Width = 17
    VertScrollBarStyles.ButtonSize = 16
    VertScrollBarStyles.UseSystemMetrics = False
    VertScrollBarStyles.ThemeMode = ttmNone
    ThemeMode = ttmNone
    BkColor = clGray
    OnDragDrop = tvPartDragDrop
    OnDragOver = tvPartDragOver
    OnStartDrag = tvPartStartDrag
  end
  object edPart3Name: TEdit
    Left = 71
    Top = 226
    Width = 107
    Height = 16
    AutoSize = False
    Color = 10526880
    TabOrder = 5
  end
  object tvPart4: TElTree
    Left = 224
    Top = 246
    Width = 217
    Height = 185
    AlwaysKeepFocus = True
    DoInplaceEdit = False
    DragAllowed = True
    DrawFocusRect = False
    FocusedSelectColor = 10526880
    FocusedSelectTextColor = clBlack
    HeaderHeight = 19
    HeaderHotTrack = False
    HeaderSections.Data = {F1FFFFFF00000000}
    HorizontalLines = True
    HorzDivLinesColor = 7368816
    HorzScrollBarStyles.Width = 17
    HorzScrollBarStyles.ButtonSize = 16
    HorzScrollBarStyles.UseSystemMetrics = False
    HorzScrollBarStyles.ThemeMode = ttmNone
    ItemIndent = 14
    LineHeight = 16
    HorzScrollTracking = True
    VertScrollTracking = True
    ShowButtons = False
    ShowCheckBoxes = True
    ShowImages = False
    ShowLines = False
    ShowRootButtons = False
    SortMode = smAdd
    StoragePath = '\Tree'
    TabOrder = 6
    Tracking = False
    TrackColor = clBlack
    VertScrollBarStyles.ShowTrackHint = True
    VertScrollBarStyles.Width = 17
    VertScrollBarStyles.ButtonSize = 16
    VertScrollBarStyles.UseSystemMetrics = False
    VertScrollBarStyles.ThemeMode = ttmNone
    ThemeMode = ttmNone
    BkColor = clGray
    OnDragDrop = tvPartDragDrop
    OnDragOver = tvPartDragOver
    OnStartDrag = tvPartStartDrag
  end
  object edPart4Name: TEdit
    Left = 295
    Top = 226
    Width = 107
    Height = 16
    AutoSize = False
    Color = 10526880
    TabOrder = 7
  end
  object fsStorage: TFormStorage
    StoredValues = <>
    Left = 65529
    Top = 65526
  end
end
