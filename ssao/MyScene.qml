import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qt3D.Input 2.0
import Qt3D.Extras 2.0

Entity {
    id: sceneRoot

    Entity {
        PlaneMesh {
            id: pm
            width: 20
            height: 20
        }
        PhongMaterial {
            id: pmm
            ambient: Qt.rgba(0.3,0.3,0.3,1)
        }
        components: [ pm, pmm ]
    }

    NodeInstantiator {

        model: 200

        delegate: Entity {
            property alias tform: tform
            property color color: "green"
            CuboidMesh { id: mesh }
            PhongMaterial { id: material; ambient: color }
            Transform { id: tform; }
            components: [ mesh, material, tform ]
        }

        onObjectAdded: {
            object.tform.translation = Qt.vector3d(Math.random()*10-5, Math.random()*10-5, Math.random()*10-5)
            object.color = ["green", "red", "yellow", "blue"][index % 4]
        }
    }

}

