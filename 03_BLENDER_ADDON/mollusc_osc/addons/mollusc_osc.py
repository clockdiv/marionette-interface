bl_info = {
    "name": "mollusc motion osc",
    "author": "clockdiv",
    "version": (0, 1),
    "blender": (4, 2, 0),
    "category": "Interface",
    "location": "Graph Editor -> Sidebar -> mollusc motion",
    "description": "",
    "warning" : "",
    "doc_url": "",
    "tracker_url": ""
}

import bpy
# from pythonosc import udp_client
from pythonosc.dispatcher import Dispatcher
from pythonosc.osc_server import BlockingOSCUDPServer
 

stop_modal = False
modal_running = False


#                   Modal Operators

class CONTROL_OT_Modal_XInput2(bpy.types.Operator):
    """Move an object with the mouse, example"""
    bl_idname = "control.xinput2"
    bl_label = "Simple Modal Operator"

    first_mouse_x: bpy.props.IntProperty()
    first_value: bpy.props.FloatProperty()

    def modal(self, context, event):
        if event.type == 'MOUSEMOVE':
            delta = self.first_mouse_x - event.mouse_x
            context.object.location.x = self.first_value + delta * 0.01

        elif event.type == 'LEFTMOUSE':
            return {'FINISHED'}

        elif event.type in {'RIGHTMOUSE', 'ESC'}:
            context.object.location.x = self.first_value
            return {'CANCELLED'}

        
        return {'RUNNING_MODAL'}

    def invoke(self, context, event):
        if context.object:
            self.first_mouse_x = event.mouse_x
            self.first_value = context.object.location.x

            context.window_manager.modal_handler_add(self)
            return {'RUNNING_MODAL'}
        else:
            self.report({'WARNING'}, "No active object, could not finish")
            return {'CANCELLED'}

class CONTROL_OT_Modal_XInput(bpy.types.Operator):
    """Modal Timer Example"""
    bl_idname = "control.xinput"
    bl_label = "Simple Modal Operator"

    # first_mouse_x: IntProperty()
    # first_value: FloatProperty()

    _timer = None

    def modal(self, context, event):
        global stop_modal
        global modal_running

        if event.type == 'TIMER':
            print(self._timer)

        if event.type == 'MOUSEMOVE':
            print('mouse moved')

        if stop_modal == True:
            modal_running = False
            wm = context.window_manager
            wm.event_timer_remove(self._timer)
            return{'FINISHED'}
        
        return{'PASS_THROUGH'}
    
    def execute(self, context):
        global stop_modal
        global modal_running
        if modal_running == False:
            stop_modal = False
            modal_running = True

            wm = context.window_manager
            # self._timer = wm.event_timer_add(.1, window=context.window)
            wm.modal_handler_add(self)
            return{'RUNNING_MODAL'}
        
        return {'CANCELLED'}

    def cancel(self, context):
        wm = context.window_manager
        wm.event_timer_remove(self._timer)
        return {'CANCELLED'}

    # def invoke(self, context, event):
    #     if context.object:
    #         self.first_mouse_x = event.mouse_x
    #         self.first_value = context.object.location.x

    #         context.window_manager.modal_handler_add(self)
    #         return {'RUNNING_MODAL'}
    #     else:
    #         self.report({'WARNING'}, "No active object, could not finish")
    #         return {'CANCELLED'}



#                   Operators

class CONTROL_OT_MolluscMotion_Modal_XInput_Start(bpy.types.Operator):
    """Starts the modal operator"""

    bl_idname = 'molluscmotion_modal.start'
    bl_label = 'Start Modal'

    @classmethod
    def poll(cls, context):
        global modal_running
        return not modal_running
    
    def execute(self, context):
        # bpy.ops.control.xinput('INVOKE_DEFAULT')       
        bpy.ops.control.xinput('EXEC_DEFAULT')        
        return {'FINISHED'}

class CONTROL_OT_MolluscMotion_Modal_XInput_Stop(bpy.types.Operator):
    """Stops the modal operator"""

    bl_idname = 'molluscmotion_modal.stop'
    bl_label = 'Stop Modal'

    @classmethod
    def poll(cls, context):
        global modal_running
        return modal_running
    
    def execute(self, context):
        global stop_modal
        stop_modal = True

        return {'FINISHED'}




#                   Panels

class INTERFACE_PT_molluscmotion_setup(bpy.types.Panel):
    """Main panel for the 'mollusc motion' add-on"""
    bl_idname = 'HARDWARE_PT_MOLLUSC_MOTION_SETUP'
    bl_label = 'Setup'
    bl_space_type = 'GRAPH_EDITOR'
    bl_region_type = 'UI'
    bl_category = 'mollusc motion'
        
    def draw(self, context):
        col = self.layout.column()
        col.label(text='mmosc')
        col.operator(CONTROL_OT_MolluscMotion_Modal_XInput_Start.bl_idname)
        col.operator(CONTROL_OT_MolluscMotion_Modal_XInput_Stop.bl_idname)

        # col.prop(context.scene.serial_port_spaghettimonster, 'serial_port_list', text='')

classes = (
    INTERFACE_PT_molluscmotion_setup,
    CONTROL_OT_Modal_XInput,
    CONTROL_OT_Modal_XInput2,
    CONTROL_OT_MolluscMotion_Modal_XInput_Start,
    CONTROL_OT_MolluscMotion_Modal_XInput_Stop
    )

def register():
    print('hello mollusc osc')
    for cls in classes:
        bpy.utils.register_class(cls)

    # bpy.types.VIEW3D_MT_object.append(menu_func)


def unregister():
    print('goodbye mollusc osc')
    for cls in classes:
        bpy.utils.unregister_class(cls)
    # bpy.types.VIEW3D_MT_object.remove(menu_func)
