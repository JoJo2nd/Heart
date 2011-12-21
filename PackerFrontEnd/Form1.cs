using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;//for dllimport


namespace PackerFrontEnd
{
    public partial class Form1 : Form
    {
        [DllImport("kernel32.dll")]
        public static extern IntPtr LoadLibrary(string dllToLoad);

        [DllImport("kernel32.dll")]
        public static extern IntPtr GetProcAddress(IntPtr hModule, string procedureName);


        [DllImport("kernel32.dll")]
        public static extern bool FreeLibrary(IntPtr hModule);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void TestDllCall();

        public Form1()
        {
            InitializeComponent();

            //IntPtr dll =  LoadLibrary("C:/Dev/Heart.v2/Heart/Debug/TextureResourceBuilder.dll");
            //IntPtr method = GetProcAddress(dll, "GetTestParam");

            //TestDllCall multiplyByTen = (TestDllCall)Marshal.GetDelegateForFunctionPointer(method,
            //                                                                            typeof(TestDllCall));
            
            //String[] toPass = new String[] { "Hello", "From", "DLL!" };
            //Resource.Param meh = multiplyByTen();

            //FreeLibrary(dll);
        }

        public void OnDragAndDrop(object sender, DragEventArgs e)
        {
            if ( e.Data.GetDataPresent( new TreeNode().GetType()))
            {
                TreeNode node = (TreeNode)e.Data.GetData(new TreeNode().GetType());
                //node.Remove();
                //treeView1.SelectedNode.Nodes.Add( node );
                ActionStack.Instance().PushAction( new ActionTreeViewMove( treeView1.SelectedNode, node, treeView1 ) );
            }
            else
            {
                // Extract the data from the DataObject-Container into a string list
                string[] FileList = (string[])e.Data.GetData(DataFormats.FileDrop, false);

                // Do something with the data...

                // For example add all files into a simple label control:
                string filenames = "";
                foreach (string File in FileList)
                    filenames += File + "\n";

                MessageBox.Show("Drag & Drop Files: " + filenames);
            }
        }

        private void treeView1_DragEnter(object sender, DragEventArgs e)
        {
            // Check if the Dataformat of the data can be accepted
            // (we only accept file drops from Explorer, etc.)
            if (e.Data.GetDataPresent(DataFormats.FileDrop))
                e.Effect = DragDropEffects.Copy; // Okay
            else if (sender == this.treeView1)
                e.Effect = DragDropEffects.Move;
            else
                e.Effect = DragDropEffects.None;
        }

        private void treeView1_ItemDrag(object sender, ItemDragEventArgs e)
        {
            if ( e.Button == MouseButtons.Left )
            {
                treeView1.DoDragDrop(e.Item, DragDropEffects.Move);
            }
            
        }

        private void addFolderToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ActionStack.Instance().PushAction(new ActionAddFolder("New Folder", treeView1.SelectedNode, treeView1));
        }

        private void treeView1_DragOver(object sender, DragEventArgs e)
        {
            // Retrieve the client coordinates of the mouse position.
            Point targetPoint = treeView1.PointToClient(new Point(e.X, e.Y));

            // Select the node at the mouse position.
            treeView1.SelectedNode = treeView1.GetNodeAt(targetPoint);
        }

        private void treeView1_KeyDown(object sender, KeyEventArgs e)
        {
            if ( e.KeyCode == Keys.Delete )
            {
                //TODO: move to action
                //treeView1.SelectedNode.Remove();
            }
        }

        private void undoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //Undo... :)
            ActionStack.Instance().PopAction();
        }
    }
}
