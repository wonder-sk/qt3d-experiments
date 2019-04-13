import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qt3D.Input 2.0
import Qt3D.Extras 2.0

Entity {
    id: sceneRoot

    property var layerPrePass   // only entities in this layer will be considered for edge detection

    Entity {

        SphereMesh {
            id: sm
        }
        MainMaterial {
            id: smm
            ambient: "red"
        }
        Transform {
            id: smt
            scale: 0.5
        }

        components: [ sm, smm, smt, layerPrePass ]
    }

    Entity {
        PlaneMesh {
            id: pm
            width: 20
            height: 20
        }
        MainMaterial {
            id: pmm
            ambient: Qt.rgba(0.3,0.3,0.3,1)
        }
        components: [ pm, pmm ] //, layerPrePass ]
    }


    Entity {
        CuboidMesh {
            id: cm
        }
        MainMaterial {
            id: cmm
            ambient: "yellow"
        }
        Transform {
            id: cmt
            translation: Qt.vector3d(0.8,0,0)
        }

        components: [ cm,  cmm, cmt, layerPrePass ]
    }

    NodeInstantiator {

        model: 200

        delegate: Entity {
            property alias tform: tform
            property color color: "green"
            CuboidMesh { id: mesh }
            MainMaterial { id: material; ambient: color }
            Transform { id: tform; }
            components: [ mesh, material, tform, layerPrePass ]
        }

        onObjectAdded: {
            object.tform.translation = Qt.vector3d(Math.random()*10-5, Math.random()*10-5, Math.random()*10-5)
            object.color = ["green", "red", "yellow", "blue"][index % 4]
        }
    }


}

