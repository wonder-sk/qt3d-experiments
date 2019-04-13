
import QtQuick 2.1 as QQ2
import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qt3D.Input 2.0
import Qt3D.Extras 2.0


RenderTarget {
    id: rt

    property int w:  1024
    property int h:  1024

    property alias textureNormal: normalAttachment
    property alias textureDepth: depthAttachment

    attachments: [
        RenderTargetOutput {
            objectName : "normal"
            attachmentPoint : RenderTargetOutput.Color0
            texture : Texture2D {
                id : normalAttachment
                width : rt.w
                height : rt.h
                format : Texture.RGB16F
                generateMipMaps : false
                magnificationFilter : Texture.Linear
                minificationFilter : Texture.Linear
                wrapMode {
                    x: WrapMode.ClampToEdge
                    y: WrapMode.ClampToEdge
                }
            }
        },
        RenderTargetOutput {
            objectName : "depth"
            attachmentPoint : RenderTargetOutput.Depth
            texture : Texture2D {
                id : depthAttachment
                width : rt.w
                height : rt.h

                format: Texture.DepthFormat
                // TODO: needed?
                comparisonFunction: Texture.CompareLessEqual
                comparisonMode: Texture.CompareRefToTexture

                generateMipMaps : false
                magnificationFilter : Texture.Linear
                minificationFilter : Texture.Linear
                wrapMode {
                    x: WrapMode.ClampToEdge
                    y: WrapMode.ClampToEdge
                }
            }
        }
    ]
}
