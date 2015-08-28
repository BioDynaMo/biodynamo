package ini.cx3d.graphics;

/**
 * This class is used when run in headless (= no gui) mode
 * It replaces function calls to perform no operation
 */
public class HeadlessViewMock extends View {

    @Override
    public void repaint() {

    }
}
