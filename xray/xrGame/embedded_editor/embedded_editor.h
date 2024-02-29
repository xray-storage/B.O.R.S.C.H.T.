#pragma once

#include <functional>

/// ������ ����������
enum class Editors {
    None,
    Details, //!< �������� ����� �����
    PpList, //!< ������ �������������
    PpEdit, //!< �������� ������������
    EnvModList, //!< ������ Env Mod
    EnvModEdit, //!< �������� Env Mod
    Infoportions, //!< ����������, ���������� ��
};

struct Editor {
public:
    Editors type() { return m_type; }

    virtual ~Editor() = 0 { }

    virtual bool draw() = 0;

protected:
    Editor(Editors type)
        : m_type(type) {};

private:
    Editors m_type = Editors::None;
};

using EditorCreator = Editor*();
struct EditorRegistrator {
    EditorRegistrator(Editors type, EditorCreator creator);
};

void switchEditor(Editors type);
void showEditor(Editor* editor);
void drawActiveEditors();
bool editorExists(std::function<bool(Editor*)> f);
